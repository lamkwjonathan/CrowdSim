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

#ifndef LIB_WORLD_BASE_H
#define LIB_WORLD_BASE_H

#include <tools/Polygon2D.h>
#include <core/agent.h>
#include <core/AgentKDTree.h>
#include <core/sph.h>
#include <core/vectorMap.h>

#include <queue>
#include <unordered_map>
typedef std::unordered_map<size_t, size_t> AgentIDMap;

/// <summary>A reference to an agent in the simulation, possibly with a transposed position.
/// Nearest-neighbor queries in WorldBase return results of this type.</summary>
struct PhantomAgent
{
private:
	/// <summary>The amount by which the neighboring Agent's position should be offset, e.g. to account for wrap-around in a toric world.</summary>
	Vector2D positionOffset;
	Vector2D position;
	float distSqr;

public:
	/// <summary>A pointer to the original Agent in the simulation.</summary>
	const Agent* realAgent;

	/// <summary>Creates a PhantomAgent with the given details.</summary>
	/// <param name="agent">A pointer to the original Agent in the simulation.</param>
	/// <param name="queryPosition">The query position used for finding neighbors; used for precomputing the distance to this PhantomAgent.</param>
	/// <param name="posOffset">The amount by which the neighboring Agent's position should be offset, e.g. to account for wrap-around in a toric world.</param>
	PhantomAgent(const Agent* agent, const Vector2D& queryPosition, const Vector2D& posOffset)
		: realAgent(agent), positionOffset(posOffset) 
	{
		UpdatePositionAndDistance(queryPosition);
	}

	PhantomAgent() : realAgent(nullptr) {}

	/// <summary>Returns the (precomputed) position of this neighboring agent, translated by the offset of this PhantomAgent.</summary>
	inline Vector2D GetPosition() const { return position; }
	/// <summary>Returns the velocity of this neighboring agent.</summary>
	inline Vector2D GetVelocity() const { return realAgent->getVelocity(); }
	/// <summary>Returns the (precomputed) squared distance from this PhantomAgent to the query position that was used to find it.</summary>
	inline float GetDistanceSquared() const { return distSqr; }
	/// <summary>Returns the mass of this neighboring agent.</summary>
	inline float GetMass() const { return realAgent->getMass(); }
	/// <summary>Returns the SPH density of this neighboring agent.</summary>
	inline float GetDensity() const { return realAgent->getDensity(); }
	/// <summary>Returns the SPH pressure of this neighboring agent.</summary>
	inline float GetPressure() const { return realAgent->getPressure(); }

	/// <summary>Pre-computes (or re-computes) the translated position of this PhantomAgent, as well as its distance to a query point.
	/// Use this method if you want to correct the PhantomAgent's data in a new frame, without having search the AgentKDTree again.</summary>
	inline void UpdatePositionAndDistance(const Vector2D& queryPosition)
	{
		position = realAgent->getPosition() + positionOffset;
		distSqr = distanceSquared(queryPosition, position);
	}

};

/// <summary>An abstract class describing a world in which a simulation can take place.</summary>
class WorldBase
{
public:

	/// <summary>An enum containing the types of world in which a simulation takes place.</summary>
	enum Type { UNKNOWN_WORLD_TYPE, INFINITE_WORLD, TORIC_WORLD };
	static Type StringToWorldType(const std::string& type);

	/// <summary>An enum containing the types of integration that a simulation uses.</summary>
	enum Integration_Mode { UNKNOWN, EULER, RK4, VERLET2, LEAPFROG2 };
	static Integration_Mode StringToIntegrationMode(const std::string& mode);

private:

	typedef std::pair<Agent*, double> AgentTimePair; 
	struct AgentTimePairComparator
	{
		bool operator()(const AgentTimePair& a, const AgentTimePair& b)
		{
			return a.second > b.second;
		}
	};

	typedef std::priority_queue<AgentTimePair, std::vector<AgentTimePair>, AgentTimePairComparator> AgentQueue;
	
	/// <summary>A list of agents (sorted by time) that will be added to the simulation in the future.</summary>
	AgentQueue agentsToAdd;

	/// <summary>A mapping from agent IDs to positions in the agents_ list.</summary>
	/// <remarks>Because agents can be removed during the simulation, the ID of an agent is not necessarily the same 
	/// as its position in the list. This is why we need this extra administration.
	/// (We could also put all agents directly in a map, but then we could not loop over the agents in parallel with OpenMP.)</remarks>
	AgentIDMap agentPositionsInVector;

	/// <summary>The agent ID that will be used for the next agent that gets added.</summary>
	size_t nextUnusedAgentID;

protected:
	 
	/// <summary>The type of this world, e.g. infinite or toric.</summary>
	const Type type_;

	/// <summary>The integration mode of this world, e.g. Semi-Implicit Euler, RK4 or SI.</summary>
	Integration_Mode mode_;

	/// <summary>A list containing the path vector maps associated with this world.</summary>
	std::vector<vectorMap*> maps_;

	/// <summary>A list containing all obstacles that are currently in the world.</summary>
	std::vector<Polygon2D> obstacles_;

	/// <summary>A list containing all agents that are currently in the crowd.</summary>
	std::vector<Agent*> agents_;

	/// <summary>A kd-tree of agent positions, used for nearest-neighbor queries.</summary>
	AgentKDTree* agentKDTree;

	/// <summary>The length (in seconds) of a fine (physics) simulation step.</summary>
	float fine_delta_time_ = 0.05; // lambda * radius / max_speed = 0.4 * 0.24 * 2 / 1.8 = 0.096 (CFL Stability Criteria)

	/// <summary>The length (in seconds) of a coarse (regular) simulation step.</summary>
	float coarse_delta_time_;

	/// <summary>The simulation time (in seconds) that has passed so far.</summary>
	double time_;

	/// <summary>The simulation time (in seconds) that has passed since the previous coarse simulation step.</summary>
	double coarse_time_;

	/// <summary>The world coordinate offset.</summary>
	Vector2D offset_ = Vector2D(0.0f,0.0f);

	/// <summary>The world width.</summary>
	int width_ = 0;

	/// <summary>The world height.</summary>
	int height_ = 0;

	/// <summary>Boolean value indicating whether global navigation is to be used in this simulation.</summary>
	bool isActiveGlobalNav_ = false;

	/// <summary>Boolean value indicating whether global navigation to nearest goal is to be used in this simulation.</summary>
	bool isActiveNearestNav_ = false;

	/// <summary>Boolean value indicating whether dynamic navigation to goal with shortest time-to-goal is to be used in this simulation.</summary>
	bool isActiveDynamicNav_ = false;

	/// <summary>Float value indicating the size of the dynamic navigation distance penalty moving average window in seconds.</summary>
	float dynamicNavTimeWindow_ = 5.0f;

	/// <summary>The SPH instance attached to this world.</summary>
	SPH sph_;

	/// <summary>Boolean value indicating whether to use SPH in this simulation.</summary>
	bool isActiveSPH_ = false;

	/// <summary>Boolean value indicating whether to use density-based blending along with SPH in this simulation.</summary>
	bool isActiveDensityBlending_ = false;

	/// <summary>Float value indicating radius around goal point (as multiplier of agent radius) that results in deletion of agent provided agent.remove_at_goal_ is set.</summary>
	float goalRadius_ = 1.0;
	
public:

#pragma region [Basic getters]
	/// @name Basic getters
	/// Methods that directly return a value stored in the world.
	/// @{

	/// <summary>Returns the list of maps. Use this to iterate all maps in arbitrary order.</summary>
	/// <returns>A non-mutable reference to the list of Map objects.</returns>
	inline const std::vector<vectorMap*>& GetMaps() const { return maps_; }

	/// <summary>Returns the list of agents. Use this if you want to retrieve information from all agents in an arbitrary order.</summary>
	/// <returns>A non-mutable reference to the list of Agent objects.</returns>
	inline const std::vector<Agent*>& GetAgents() const { return agents_; }

	/// <summary>Returns the list of obstacles.</summary>
	/// <returns>A non-mutable reference to the list of Polygon2D objects, each representing one obstacle in the world.</returns>
	inline const std::vector<Polygon2D>& GetObstacles() const { return obstacles_; }

	/// <summary>Returns the current simulation time (in seconds).</summary>
	/// <returns>The time (in seconds) that has been simulated since the simulation started.</returns>
	inline double GetCurrentTime() const { return time_; }

	/// <summary>Returns the current simulation time since previous coarse step(in seconds).</summary>
	/// <returns>The time (in seconds) that has been simulated since the previous coarse step.</returns>
	inline double GetCurrentCoarseTime() const { return coarse_time_; }

	/// <summary>Returns the duration of a single fine simulation time step (in seconds), i.e. the time that is simulated in a single execution of DoStep().</summary>
	/// <returns>The duration of a single fine simulation time step (in seconds).</returns>
	inline float GetFineDeltaTime() const { return fine_delta_time_; }

	/// <summary>Returns the duration of a single coarse simulation time step (in seconds), i.e. the time that is simulated in a single execution of ComputeNeighbours() and some RVO calculations.</summary>
	/// <returns>The duration of a single coarse simulation time step (in seconds).</returns>
	inline float GetCoarseDeltaTime() const { return coarse_delta_time_; }

	/// <summary>Returns the type of this world, i.e. infinite or toric.</summary>
	/// <returns>The value of the Type enum describing the type of this world.</returns>
	inline Type GetType() { return type_; }

	/// <summary>Returns the integration mode of this world, i.e. Semi-Implicit Euler, RK4 or SI.</summary>
	/// <returns>The value of the Integration_Mode enum describing the integration mode.</returns>
	inline Integration_Mode GetMode() { return mode_; }

	/// <summary>Returns the offset of this world.</summary>
	/// <returns>A Vector2D referencing the offset of this world with respect to global Geo-coordinates.</returns>
	inline Vector2D* GetOffset() { return &offset_; }

	/// <summary>Returns the width of this world.</summary>
	/// <returns>An int referencing the width of this world.</returns>
	inline int GetWidth() const { return width_; }

	/// <summary>Returns the width of this world.</summary>
	/// <returns>An int referencing the width of this world.</returns>
	inline int GetHeight() const { return height_; }

	/// <summary>Returns the boolean isActiveGlobalNav that indicates whether global navigation is used in this simulation.</summary>
	/// <returns>A boolean denoting whether global navigation is used.</returns>
	inline bool GetIsActiveGlobalNav() { return isActiveGlobalNav_; }

	/// <summary>Returns the boolean isActiveNearestNav that indicates whether global navigation to nearest goal is used in this simulation.</summary>
	/// <returns>A boolean denoting whether global navigation to nearest goal is used.</returns>
	inline bool GetIsActiveNearestNav() { return isActiveNearestNav_; }

	/// <summary>Returns the boolean isActiveDynamicNav that indicates whether dynamic navigation is used in this simulation.</summary>
	/// <returns>A boolean denoting whether dynamic navigation is used.</returns>
	inline bool GetIsActiveDynamicNav() { return isActiveDynamicNav_; }

	/// <summary>Returns the float value denoting the size of the dynamic navigation distance penalty moving average window in seconds.</summary>
	/// <returns>A float denoting the size of dynamic navigation distance penalty moving average window in seconds.</returns>
	inline float GetDynamicNavTimeWindow() { return dynamicNavTimeWindow_; }

	/// <summary>Returns the SPH instance attached to this world.</summary>
	/// <returns>A reference to the SPH instance.</returns>
	inline SPH* GetSPH() { return &sph_; }

	/// <summary>Returns the boolean isActiveSPH that indicates whether SPH is used in this simulation.</summary>
	/// <returns>A boolean denoting whether SPH is to be used.</returns>
	inline bool GetIsActiveSPH() { return isActiveSPH_; }

	/// <summary>Returns the boolean isActiveDensityBlending that indicates whether density-based blending is used along with SPH in this simulation.</summary>
	/// <returns>A boolean denoting whether density-based blending is to be used.</returns>
	inline bool GetIsActiveDensityBlending() { return isActiveDensityBlending_; }

	/// <summary>Returns the goal radius of this world.</summary>
	/// <returns>A float value indicating the radius around the goal point (as a multiplier of agent radius) where agent will be removed upon arriving at.</returns>
	inline float GetGoalRadius() { return goalRadius_; }

	/// @}
#pragma endregion

#pragma region [Basic setters]
	/// @name Basic setters
	/// Methods that directly change the world's internal status.
	/// @{

	/// <summary>Sets the number of parallel threads that this class may use for the simulation.</summary>
	/// <remarks>The given number is sent to OpenMP. Whether this number can be used depends on the numer of (virtual) cores of your machine.</remarks>
	/// <param name="nrThreads">The desired number of threads to use.</param>
	void SetNumberOfThreads(int nrThreads);

	/// <summary>Sets the length of fine simulation time steps.</summary>
	/// <param name="fine_delta_time">The desired length (in seconds) of a single fine simulation frame.</param>
	inline void SetFineDeltaTime(float fine_delta_time) { fine_delta_time_ = fine_delta_time; }
	
	/// <summary>Sets the length of coarse simulation time steps.</summary>
	/// <param name="coarse_delta_time">The desired length (in seconds) of a single coarse simulation frame.</param>
	inline void SetCoarseDeltaTime(float coarse_delta_time) { coarse_delta_time_ = coarse_delta_time; }

	/// <summary>Sets the integration mode of this world, i.e. Semi-Implicit Euler, RK4 or SI.</summary>
	/// <maram name="mode">The value of the Integration_Mode enum describing the integration mode.</returns>
	inline void SetMode(Integration_Mode mode) { mode_ = mode; }

	/// <summary>Sets the offset of this world from Geo-coordinates.</summary>
	/// <maram name="offset">The value to offset all objects in this world by.</returns>
	inline void SetOffset(Vector2D offset) { offset_ = offset; }

	/// <summary>Sets the width of this world.</summary>
	/// <maram name="width">The value representing the width of the world.</returns>
	inline void SetWidth(int width) { width_ = width; }

	/// <summary>Sets the height of this world.</summary>
	/// <maram name="height">The value representing the height of the world.</returns>
	inline void SetHeight(int height) { height_ = height; }

	/// <summary>Sets whether global navigation is used in the simulation.</summary>
	/// <param name="b">The boolean indicating whether global navigation is used.</param>
	inline void SetIsActiveGlobalNav(bool b) { isActiveGlobalNav_ = b; }

	/// <summary>Sets whether global navigation to nearest goal is used in the simulation.</summary>
	/// <param name="b">The boolean indicating whether global navigation to nearest goal is used.</param>
	inline void SetIsActiveNearestNav(bool b) { isActiveNearestNav_ = b; }

	/// <summary>Sets whether dynamic navigation is used in the simulation.</summary>
	/// <param name="b">The boolean indicating whether dynamic navigation is used.</param>
	inline void SetIsActiveDynamicNav(bool b) { isActiveDynamicNav_ = b; }

	/// <summary>Sets whether SPH is used in the simulation.</summary>
	/// <param name="b">The boolean indicating whether SPH is used.</param>
	inline void SetIsActiveSPH(bool b) { isActiveSPH_ = b; }

	/// <summary>Sets whether density-based blending is used in the simulation.</summary>
	/// <param name="b">The boolean indicating whether density-based blending is used.</param>
	inline void SetIsActiveDensityBlending(bool b) { isActiveDensityBlending_ = b; }

	/// <summary>Sets the goal radius of this world.</summary>
	/// <param name="b">The radius around goal point (as a multiplier of agent radius) where agent will be removed upon arriving at.</param>
	inline void SetGoalRadius(float radius) { goalRadius_ = radius; }

	/// @}
#pragma endregion

	/// <summary>Performs a single iteration of the simulation. 
	/// All agents compute their new desired velocity and then move forward.</summary>
	void DoStep();
	
	/// <summary>Computes and returns a list of all agents that are within a given radius of a given position.</summary>
	/// <remarks>Subclasses of WorldBase must implement this method, because the result may depend on special properties (e.g. the wrap-around effect in WorldToric).</remarks>
	virtual NeighborList ComputeNeighbors(const Vector2D& position, float search_radius, const Agent* queryingAgent) const = 0;

#pragma region [Finding, adding, and removing agents]
	/// @name Finding, adding, and removing agents
	/// Methods for finding, adding, and removing agents in the simulation.
	/// @{

	/// <summary>Finds and returns the agent with the given ID, if it exists.</summary>
	/// <param name="id">The ID of the agent to find.</param>
	/// <returns>A mutable pointer to the Agent stored under the given ID, or nullptr if this agent does not exist.</returns>
	Agent* GetAgent(size_t id);

	/// <summary>Finds and returns the agent with the given ID, if it exists.</summary>
	/// <param name="id">The ID of the agent to find.</param>
	/// <returns>A non-mutable pointer to the Agent stored under the given ID, or nullptr if this agent does not exist.</returns>
	const Agent* GetAgent_const(size_t id) const;

	/// <summary>Creates a new Agent object to be added to the simulation at the given time.</summary>
	/// <remarks>If the given time has already been reached, the agent will be added to the simulation immediately. 
	/// Otherwise, the agent will be scheduled for insertion at the given moment.</remarks>
	/// <param name="position">The start position of the agent.</param>
	/// <param name="settings">The simulation settings of the agent.</param>
	/// <param name="desiredID">(optional) The desired ID of the agent. 
	/// If it is not set, or if this ID is already taken, a suitable ID will be chosen automatically.</param>
	/// <param name="startTime">(optional) The simulation time at which the agent should be added. 
	/// If it is not set (or set to a value lower than the current simulation time), the agent will be added immediately.
	/// Otherwise, the agent will be scheduled for insertion at the given moment.</param>
	/// <returns>A pointer to the newly created Agent object. The agent may have a different ID than the desired one.
	/// Also, the WorldBase class manages the memory of agents, so you do not have to delete this object yourself.</returns>
	Agent* AddAgent(const Vector2D& position, const Agent::Settings& settings, size_t desiredID = std::numeric_limits<size_t>::max(), float startTime = 0);

	/// <summary>Tries to remove the agent with the given ID from the simulation.</summary>
	/// <param name="id">The ID of the agent to remove.</param>
	/// <returns>true if the agent was successfully removed; false otherwise, i.e. if the agent with the given ID does not exist.</returns>
	bool RemoveAgent(size_t id);

	/// @}
#pragma endregion

	/// <summary>Adds a map to the world.</summary>
	/// <param name="m">A vectorMap containing information on global path to a goal for this world.</param>
	virtual void AddMap(vectorMap* m);

	/// <summary>Adds an obstacle with the given vertices to the world.</summary>
	/// <param name="points">A sequence of 2D points defining the obstacle's boundary vertices.</param>
	virtual void AddObstacle(const std::vector<Vector2D>& points);

	/// <summary>Cleans up this WorldBase object for removal.</summary>
	virtual ~WorldBase();

protected:

	/// <summary>Creates a WorldBase object of the given type.</summary>
	WorldBase(Type type);

	/// <summary>Computes a list of all agents that lie within a given radius of a given position.</summary>
	/// <param name="position">A query position.</param>
	/// <param name="search_radius">A query radius.</param>
	/// <param name="queryingAgent">A pointer to the Agent object performing the query. This agent will be excluded from the results.
	/// Use nullptr to not exclude any agents.</param>
	/// <param name="result">[out] Will store a list of pointers to agents queryingAgent lie within "search_radius" meters of "position", 
	/// excluding the agent denoted by "queryingAgent" (if it exists).</param>
	void computeNeighboringAgents_Flat(const Vector2D& position, float search_radius, const Agent* queryingAgent, std::vector<const Agent*>& result) const;

	void computeNeighboringObstacles_Flat(const Vector2D& position, float search_radius, std::vector<LineSegment2D>& result) const;

	/// <summary>Subroutine of DoStep() that moves all agents forward using their last computed "new velocities".</summary>
	/// <remarks>Subclasses of WorldBase may override this method if they require special behavior (e.g. the wrap-around effect in WorldToric).</remarks>
	virtual void DoStep_MoveAllAgents();

private:
	/// Adds a (previously created) agent to the simulation.
	void addAgentToList(Agent* agent);

	/// Removes the agent at a given position in the list, 
	/// and does the necessary management to keep this list valid.
	void removeAgentAtListIndex(size_t index);
};

#endif //LIB_WORLD_BASE_H
