/* UMANS: Unified Microscopic Agent Navigation Simulator
** MIT License
** Copyright (C) 2018-2020  Inria Rennes Bretagne Atlantique - Rainbow - Julien Pettré
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject
** to the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
** Contact: crowd_group@inria.fr
** Website: https://project.inria.fr/crowdscience/
** See the file AUTHORS.md for a list of all contributors.
*/

#include <core/worldBase.h>
#include <omp.h>

using namespace nanoflann;
using namespace std;

WorldBase::Type WorldBase::StringToWorldType(const std::string& type)
{
	if (type == "Infinite")
		return Type::INFINITE_WORLD;
	else if (type == "Toric")
		return Type::TORIC_WORLD;
	else
		return Type::UNKNOWN_WORLD_TYPE;
}

WorldBase::Integration_Mode WorldBase::StringToIntegrationMode(const std::string& mode)
{
	if (mode == "Euler")
		return Integration_Mode::EULER;
	else if (mode == "RK4")
		return Integration_Mode::RK4;
	else if (mode == "Verlet")
		return Integration_Mode::VERLET2;
	else if (mode == "Leapfrog")
		return Integration_Mode::LEAPFROG2;
	else
		return Integration_Mode::UNKNOWN;
}

WorldBase::WorldBase(WorldBase::Type type) : type_(type)
{
	time_ = 0;
	coarse_time_ = 0;
	agentKDTree = nullptr;
	SetNumberOfThreads(1);
	nextUnusedAgentID = 0;
}

void WorldBase::SetNumberOfThreads(int nrThreads)
{
	omp_set_num_threads(nrThreads);
}

void WorldBase::computeNeighboringAgents_Flat(const Vector2D& position, float search_radius, const Agent* queryingAgent, std::vector<const Agent*>& result) const
{
	// get the IDs of neighbors
	const auto& agentIDs = agentKDTree->FindAllAgentsInRange(position, search_radius, queryingAgent);

	// convert them to agent pointers
	result.resize(agentIDs.size());
	for (int i = 0; i < agentIDs.size(); ++i)
		result[i] = GetAgent_const(agentIDs[i]);
}

void WorldBase::computeNeighboringObstacles_Flat(const Vector2D& position, float search_radius, std::vector<LineSegment2D>& result) const
{
	// TODO: implement more efficient neighbor search for obstacles
	const float radiusSquared = search_radius * search_radius;

	// loop over all obstacle edges
	for (const auto& obs : obstacles_)
	{
		for (const auto& edge : obs.GetEdges())
		{
			// if this line segment is clsoe enough, add it to the result
			if (distanceToLineSquared(position, edge.first, edge.second, true) <= radiusSquared)
				result.push_back(edge);
		}
	}
}

void WorldBase::DoStep()
{
	int n;
	if (coarse_time_ == 0.0f)
	{
		// Before the simulation frame begins, add agents that need to be added now (once every coarse time step)
		while (!agentsToAdd.empty() && agentsToAdd.top().second <= time_)
		{
			addAgentToList(agentsToAdd.top().first);
			agentsToAdd.pop();
		}
		n = (int)agents_.size();

		// --- Main simulation tasks:
		// 1. build the KD tree for nearest-neighbor computations (once every coarse time step)
		if (agentKDTree != nullptr)
			delete agentKDTree;
		agentKDTree = new AgentKDTree(agents_);

		// 2. compute nearest neighbors for each agent (once every coarse time step)
		// Seems to be inefficient to step into agent only to step out again (might want to refactor)
		#pragma omp parallel for 
		for (int i = 0; i < n; ++i)
			agents_[i]->ComputeNeighbors(this);
	}
	else
	{
		n = (int)agents_.size();
		#pragma omp parallel for 
		for (int i = 0; i < n; ++i)
			agents_[i]->UpdateNeighbors(this);
	}

	// compute SPH parameters if required
	if (GetIsActiveSPH()) 
	{
		#pragma omp parallel for 
		for (int i = 0; i < n; ++i)
			agents_[i]->ComputeBaseSPH(this);

		#pragma omp parallel for 
		for (int i = 0; i < n; ++i)
			agents_[i]->ComputeDerivedSPH(this);
	}
	
	// 3. compute a preferred velocity for each agent
	#pragma omp parallel for 
	for (int i = 0; i < n; ++i)
		agents_[i]->ComputePreferredVelocity(this);

	// 4. perform local navigation for each agent, to compute an acceleration vector for them (once every coarse time step for velocity-based methods)
	#pragma omp parallel for 
	for (int i = 0; i < n; ++i)
		agents_[i]->ComputeAcceleration(this);

	// 5. compute contact forces for all agents
	#pragma omp parallel for 
	for (int i = 0; i < n; ++i)
		agents_[i]->ComputeContactForces(this);

	// 6. move all agents to their new positions
	DoStep_MoveAllAgents();

	// update map parameters if required
	if (isActiveGlobalNav_ && isActiveNearestNav_ && isActiveDynamicNav_)
	{
		// Parallelized with buckets
		#pragma omp parallel for 
		for (int i = 0; i < n; i++)
		{
			agents_[i]->UpdateMapParameters(this);
		}
		float ratio = fine_delta_time_ / GetDynamicNavTimeWindow();
		for (int i = 0; i < maps_.size(); ++i)
		{
			// Collate thread-local parameters
			maps_[i]->collateThreadLocalVariables();

			// For using congestion value only with exponential moving average
			//maps_[i]->setDistanceMultiplier((1 - ratio) * maps_[i]->getDistanceMultiplier() + ratio * vectorMap::multiplierFromCongestionValue(maps_[i]->getCongestionValue() / maps_[i]->getWeightedCount()));
			
			// For using congestion value only without exponential moving average
			//maps_[i]->setDistanceMultiplier(vectorMap::multiplierFromCongestionValue(maps_[i]->getCongestionValue() / maps_[i]->getWeightedCount()));

			// For using speed value only with exponential moving average
			maps_[i]->setDistanceMultiplier((1 - ratio) * maps_[i]->getDistanceMultiplier() + ratio * maps_[i]->getAgentCount() / maps_[i]->getSpeedValue());
			
			// For using speed value only without exponential moving average
			//maps_[i]->setDistanceMultiplier(maps_[i]->getAgentCount() / maps_[i]->getSpeedValue());

			// For using congestion value + speed value with exponential moving average
			//maps_[i]->setDistanceMultiplier((1 - ratio) * maps_[i]->getDistanceMultiplier() + ratio * 0.5 * (1.4 * maps_[i]->getAgentCount() / maps_[i]->getSpeedValue() + vectorMap::multiplierFromCongestionValue(maps_[i]->getCongestionValue() / maps_[i]->getWeightedCount())));
			
			// For using congestion value + speed value with exponential moving average
			//maps_[i]->setDistanceMultiplier(0.5 * (1.4 * maps_[i]->getAgentCount() / maps_[i]->getSpeedValue() + vectorMap::multiplierFromCongestionValue(maps_[i]->getCongestionValue() / maps_[i]->getWeightedCount())));

			maps_[i]->setWeightedCount(1.0f);
			maps_[i]->setCongestionValue(1.0f);
			maps_[i]->setAgentCount(1);
			maps_[i]->setSpeedValue(1.4f);
		}
	}
	// --- End of main simulation tasks.	

	// increase the time that has passed
	time_ += fine_delta_time_;
	coarse_time_ += fine_delta_time_;
	if (coarse_time_ >= coarse_delta_time_)
		coarse_time_ = 0.0f;

	// remove agents who have reached their goal
	// cannot parallelize without causing loop error
	for (int i = n - 1; i >= 0; --i)
		if (agents_[i]->getRemoveAtGoal() && agents_[i]->hasReachedGoal(GetGoalRadius()))
			removeAgentAtListIndex(i);
}

void WorldBase::DoStep_MoveAllAgents()
{
	if (mode_ == WorldBase::Integration_Mode::EULER)
	{
		#pragma omp parallel for 
		for (int i = 0; i < (int)agents_.size(); ++i)
			agents_[i]->UpdateVelocityAndPosition(this);
	}
	else if (mode_ == WorldBase::Integration_Mode::RK4)
	{
		#pragma omp parallel for 
		for (int i = 0; i < (int)agents_.size(); ++i)
			agents_[i]->UpdateVelocityAndPosition_RK4(this);
	}
	else if (mode_ == WorldBase::Integration_Mode::VERLET2)
	{
		#pragma omp parallel for 
		for (int i = 0; i < (int)agents_.size(); ++i)
			agents_[i]->UpdateVelocityAndPosition_Verlet2(this);
	}
	else if (mode_ == WorldBase::Integration_Mode::LEAPFROG2)
	{
		#pragma omp parallel for 
		for (int i = 0; i < (int)agents_.size(); ++i)
			agents_[i]->UpdateVelocityAndPosition_Leapfrog2(this);
	}
	else
	{
		#pragma omp parallel for 
		for (int i = 0; i < (int)agents_.size(); ++i)
			agents_[i]->UpdateVelocityAndPosition(this);
	}
}

#pragma region [Finding, adding, and removing agents]

Agent* WorldBase::GetAgent(size_t id)
{
	// find out if the agent with this ID exists
	auto positionInList = agentPositionsInVector.find(id);
	if (positionInList == agentPositionsInVector.end())
		return nullptr;

	// return the agent who's at the correct position in the list
	return agents_[positionInList->second];
}

const Agent* WorldBase::GetAgent_const(size_t id) const
{
	// find out if the agent with this ID exists
	auto positionInList = agentPositionsInVector.find(id);
	if (positionInList == agentPositionsInVector.end())
		return nullptr;

	// return the agent who's at the correct position in the list
	return agents_[positionInList->second];
}

Agent* WorldBase::AddAgent(const Vector2D& position, const Agent::Settings& settings, size_t desiredID, float startTime)
{
	// determine the right ID for the agent
	size_t agentID;
	if (desiredID != std::numeric_limits<size_t>::max() && GetAgent(desiredID) == nullptr) // custom ID specified by the user; use it only if it's not already taken
		agentID = desiredID;
	else // determine a good ID automatically
		agentID = nextUnusedAgentID;

	// create the agent and set its position
	Agent* agent = new Agent(agentID, settings);
	agent->setPosition(position);

	// if the new ID is the highest one so far, update the next available ID
	if (agentID >= nextUnusedAgentID)
		nextUnusedAgentID = agentID + 1;

	// if desired, add the agent immediately
	if (startTime <= time_)
		addAgentToList(agent);

	// otherwise, schedule the agent for insertion at a later time
	else
		agentsToAdd.push({ agent, startTime });

	return agent;
}

void WorldBase::addAgentToList(Agent* agent)
{
	// add the agent to the list, and store where in the list it is located
	agentPositionsInVector[agent->getID()] = agents_.size();
	agents_.push_back(agent);
}

bool WorldBase::RemoveAgent(size_t id)
{
	// find out if the agent with this ID exists
	auto positionInList = agentPositionsInVector.find(id);
	if (positionInList == agentPositionsInVector.end())
		return false;

	removeAgentAtListIndex(positionInList->second);
	return true;
}

void WorldBase::removeAgentAtListIndex(size_t index)
{
	const auto removedAgentID = agents_[index]->getID();
	
	// if the agent is at the end of the list, simply remove it
	if (index + 1 == agents_.size())
	{
		agents_.pop_back();
		agentPositionsInVector.erase(removedAgentID);
	}
	
	// if the agent is not at the end of the list, do some extra management
	else
	{
		// delete the requested agent
		delete agents_[index];

		// move the last agent in the list to the position that has now become free
		auto lastAgent = agents_.back();
		agents_[index] = lastAgent;
		agents_.pop_back();

		// update the position map:
		// - the requested agent is now gone
		agentPositionsInVector.erase(removedAgentID);
		// - another agent has moved
		agentPositionsInVector[lastAgent->getID()] = index;
	}
}

#pragma endregion

void WorldBase::AddMap(vectorMap* m)
{
	maps_.push_back(m);
}

void WorldBase::AddObstacle(const std::vector<Vector2D>& points)
{
	obstacles_.push_back(Polygon2D(points));
}

WorldBase::~WorldBase()
{
	// delete the KD tree
	if (agentKDTree != nullptr)
		delete agentKDTree;

	// delete all agents
	for (vectorMap* m : maps_)
		delete m;
	maps_.clear();

	// delete all agents
	for (Agent* agent : agents_)
		delete agent;
	agents_.clear();

	// delete all agents that were scheduled for insertion
	while (!agentsToAdd.empty())
	{
		delete agentsToAdd.top().first;
		agentsToAdd.pop();
	}

	// delete the mapping from IDs to agents
	agentPositionsInVector.clear();
	nextUnusedAgentID = 0;
}