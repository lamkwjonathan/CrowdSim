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

#include <core/agent.h>
#include <core/worldBase.h>
#include <stdio.h>
#include <iostream>

Agent::Agent(size_t id, const Agent::Settings& settings) :
	id_(id), settings_(settings),
	position_(0, 0),
	velocity_(0, 0),
	acceleration_(0, 0),
	contact_forces_(0, 0),
	preferred_velocity_(0, 0),
	goal_(0, 0),
	viewing_direction_(0, 0),
	next_acceleration_(0, 0),
	next_contact_forces_(0, 0),
	sph_density_(0),
	personal_rest_density_(0),
	pressure_(0),
	pressure_force_(0, 0),
	viscosity_force_(0, 0),
	sph_acceleration_(0, 0),
	next_sph_density_(0),
	next_personal_rest_density_(0),
	next_pressure_force_(0, 0),
	next_viscosity_force_(0, 0)
	
{
	// set the seed for random-number generation
	RNGengine_.seed((unsigned int)id);

	// apply the navigation policy
	setPolicy(settings_.policy_);
}

#pragma region [Simulation-loop methods]

void Agent::ComputeNeighbors(WorldBase* world)
{
	// get the query radius
	float range = getPolicy()->getInteractionRange();

	// perform the query and store the result
	neighbors_ = world->ComputeNeighbors(position_, range, this);
}

void Agent::ComputeBaseSPH(WorldBase* world)
{
	next_sph_density_ = getMass() * world->GetSPH()->poly6_kernel(Vector2D(0,0)); // Lowest density at the agent's position is just his own mass multiplied by the smoothing kernel.

	// check all neighboring agents to tally SPH density
	for (const auto& neighborAgent : neighbors_.first)
	{
		next_sph_density_ += neighborAgent.GetMass() * world->GetSPH()->poly6_kernel(position_ - neighborAgent.GetPosition());
	}
	sph_density_ = next_sph_density_;

	// calculate personal rest density
	float ratio = world->GetFineDeltaTime() / world->GetSPH()->getDensityTimeWindow();
	next_personal_rest_density_ = (1 - ratio) * personal_rest_density_ + ratio * sph_density_;
	personal_rest_density_ = world->GetSPH()->clampPersonalRestDensity(next_personal_rest_density_);

	// calculate pressure
	pressure_ = world->GetSPH()->getGasConstant() * (sph_density_ - personal_rest_density_);
}

void Agent::ComputeDerivedSPH(WorldBase* world)
{
	// check all neighboring agents to calculate pressure force (only done if density of agent > current rest density).
	// no pressure from self since self results in magnitude of 0. Spiky kernel has division by magnitude.
	if (sph_density_ >= personal_rest_density_)
	{
		next_pressure_force_.x = 0;
		next_pressure_force_.y = 0;
		
		for (const auto& neighborAgent : neighbors_.first)
		{
			next_pressure_force_ += (neighborAgent.GetMass() * (pressure_ + neighborAgent.GetPressure()) * world->GetSPH()->spiky_kernel(position_ - neighborAgent.GetPosition()))
				/ (2 * neighborAgent.GetDensity());
		}
		pressure_force_ = next_pressure_force_;
	}
	else
	{
		pressure_force_.x = 0;
		pressure_force_.y = 0;
	}

	// check all neighboring agents to calculate pressure force (only done if viscosity constant != 0)
	// no viscosity from self since velocity_ - velocity_ results in zero.
	if (world->GetSPH()->getViscosityConstant() != 0) 
	{
		next_viscosity_force_.x = 0;
		next_viscosity_force_.y = 0;

		for (const auto& neighborAgent : neighbors_.first)
		{
			next_viscosity_force_ += (neighborAgent.GetMass() * (neighborAgent.GetVelocity() - velocity_) * world->GetSPH()->mullers_kernel(position_ - neighborAgent.GetPosition()))
				/ (neighborAgent.GetDensity());
		}
		viscosity_force_ = world->GetSPH()->getViscosityConstant() * next_viscosity_force_; 
	}
	
	// compute SPH acceleration
	sph_acceleration_ = (-pressure_force_ + viscosity_force_) / sph_density_;
}

void Agent::ComputePreferredVelocity(WorldBase* world)
{
	if (hasReachedGoal(world->GetGoalRadius()))
		preferred_velocity_ = Vector2D(0, 0);

	else
		preferred_velocity_ = (goal_ - position_).getnormalized() * getPreferredSpeed();
}

void Agent::ComputeAcceleration(WorldBase* world)
{
	if (!(getPolicy()->GetName() == "RVO" && world->GetCurrentCoarseTime() != 0.0f))
		next_acceleration_ = getPolicy()->ComputeAcceleration(this, world);
}

void Agent::ComputeContactForces(WorldBase* world)
{
	next_contact_forces_ = getPolicy()->ComputeContactForces(this, world);
}

void Agent::updateViewingDirection()
{
	// update the viewing direction:
	// weighted average of the preferred and actual velocity
	const auto& vDiff = (2 * velocity_ + preferred_velocity_) / 3;
	if (vDiff.sqrMagnitude() > 0.01)
		viewing_direction_ = vDiff.getnormalized();
}

void Agent::UpdateVelocityAndPosition(WorldBase* world)
{
	const float dt = world->GetFineDeltaTime();
	
	// add contact forces
	contact_forces_ = next_contact_forces_;

	// add and clamp the acceleration
	acceleration_ = clampVector((next_acceleration_ + sph_acceleration_ + contact_forces_ / settings_.mass_), getMaximumAcceleration());

	// integrate the velocity; clamp to a maximum speed
	velocity_ = clampVector(velocity_ + (acceleration_ * dt), getMaximumSpeed());

	// update the position
	position_ += velocity_ * dt;

	/*
	// clamp the acceleration
	acceleration_ = clampVector((next_acceleration_ + sph_acceleration_), getMaximumAcceleration());

	// integrate the velocity; clamp to a maximum speed
	velocity_ = clampVector(velocity_ + (acceleration_ * dt), getMaximumSpeed());

	// add contact forces
	contact_forces_ = next_contact_forces_;
	velocity_ += contact_forces_ / settings_.mass_ * dt;

	// update the position
	position_ += velocity_ * dt;
	*/
	updateViewingDirection();
}

#pragma endregion

#pragma region [Advanced getters]

bool Agent::hasReachedGoal(float range_multiplier) const
{
	return (goal_ - position_).sqrMagnitude() <= getRadius() * getRadius() * range_multiplier * range_multiplier;
}

float Agent::getDeltaTime(const WorldBase* world)
{
	if (getPolicy()->GetName() == "RVO")
		return world->GetCoarseDeltaTime();
	else
		return world->GetFineDeltaTime();
}

#pragma endregion

#pragma region [Basic setters]

void Agent::setPosition(const Vector2D &position)
{
	position_ = position;
}

void Agent::setVelocity_ExternalApplication(const Vector2D &velocity, const Vector2D &viewingDirection)
{
	velocity_ = velocity;
	viewing_direction_ = viewingDirection;
}

void Agent::setGoal(const Vector2D &goal)
{
	goal_ = goal;

	// look straight towards the goal
	if (goal_ != position_)
		viewing_direction_ = (goal_ - position_).getnormalized();
}

void Agent::setPolicy(Policy* policy)
{
	settings_.policy_ = policy;
}

#pragma endregion

float Agent::ComputeRandomNumber(float min, float max)
{
	auto distribution = std::uniform_real_distribution<float>(min, max);
	return distribution(RNGengine_);
}