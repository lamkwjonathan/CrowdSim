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
	original_velocity_(0, 0),
	velocity_(0, 0),
	acceleration_(0, 0),
	contact_forces_(0, 0),
	preferred_velocity_(0, 0),
	goal_(0, 0),
	viewing_direction_(0, 0),
	next_acceleration_(0, 0),
	next_contact_forces_(0, 0),
	sph_density_(1),
	personal_rest_density_(1),
	pressure_(0),
	pressure_force_(0, 0),
	viscosity_force_(0, 0),
	sph_acceleration_(0, 0),
	next_sph_density_ags_(0),
	next_sph_density_obs_(0),
	next_personal_rest_density_(0),
	next_pressure_force_ags_(0, 0),
	next_pressure_force_obs_(0, 0),
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

void Agent::UpdateNeighbors(WorldBase* world)
{
	for (auto neighbor_agent : neighbors_.first)
	{
		neighbor_agent.UpdatePositionAndDistance(position_);
	}
}

void Agent::ComputeBaseSPH(WorldBase* world)
{
	next_sph_density_ags_ = getMass() * world->GetSPH()->poly6_kernel(Vector2D(0,0)); // Lowest density at the agent's position is just his own mass multiplied by the smoothing kernel.

	// check all neighboring agents and walls to tally SPH density
	for (const auto& neighborAgent : neighbors_.first)
	{
		next_sph_density_ags_ += neighborAgent.GetMass() * world->GetSPH()->poly6_kernel(position_ - neighborAgent.GetPosition());
	}
	setColorByDensity(next_sph_density_ags_);

	next_sph_density_obs_ = 0;

	for (const auto& neighborObstacle : neighbors_.second)
	{
		float distToAgent = distanceToLine(position_, neighborObstacle.first, neighborObstacle.second, true);
		if (distToAgent <= 1)
		{
			Vector2D nearestPoint = nearestPointOnLine(position_, neighborObstacle.first, neighborObstacle.second, true);
			Vector2D representativePoint = world->GetSPH()->calcRepresentativePoint(position_, nearestPoint);
			next_sph_density_obs_ += personal_rest_density_ * world->GetSPH()->calcObstacleArea(position_, neighborObstacle) * world->GetSPH()->poly6_kernel(position_ - representativePoint);
		}

	}
	sph_density_ = next_sph_density_ags_ + next_sph_density_obs_;

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
		next_pressure_force_ags_.x = 0;
		next_pressure_force_ags_.y = 0;
		
		for (const auto& neighborAgent : neighbors_.first)
		{
			next_pressure_force_ags_ += (neighborAgent.GetMass() * (pressure_ + neighborAgent.GetPressure()) * world->GetSPH()->spiky_kernel(position_ - neighborAgent.GetPosition()))
				/ (2 * neighborAgent.GetDensity());
		}

		next_pressure_force_obs_.x = 0;
		next_pressure_force_obs_.y = 0;

		for (const auto& neighborObstacle : neighbors_.second)
		{
			Vector2D nearestPoint = nearestPointOnLine(position_, neighborObstacle.first, neighborObstacle.second, true);
			Vector2D representativePoint = world->GetSPH()->calcRepresentativePoint(position_, nearestPoint);
			next_pressure_force_obs_ += pressure_ * world->GetSPH()->calcObstacleArea(position_, neighborObstacle) * world->GetSPH()->spiky_kernel(position_ - representativePoint);
		}
		pressure_force_ = next_pressure_force_ags_ + next_pressure_force_obs_;
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
	else if (world->GetIsActiveGlobalNav())
	{
		if (position_.x >= 0 && position_.x < world->GetWidth() && position_.y >= 0 && position_.y < world->GetHeight())
		{
			preferred_velocity_ = world->vectorArray_[(int)std::floor(position_.x) + (int)std::floor(position_.y) * world->GetWidth()] * getPreferredSpeed();
		}
		else
			preferred_velocity_ = (goal_ - position_).getnormalized() * getPreferredSpeed();
	}
		
	else
		preferred_velocity_ = (goal_ - position_).getnormalized() * getPreferredSpeed();
}

void Agent::ComputeAcceleration(WorldBase* world)
{
	if (!(getPolicy()->GetName() == "RVO" && world->GetCurrentCoarseTime() != 0.0f))
		next_acceleration_ = getPolicy()->ComputeAcceleration(this, world);
}

Vector2D Agent::ComputeAcceleration_RK4(Vector2D velocity, WorldBase* world)
{
	return getPolicy()->ComputeAcceleration_RK4(this, velocity, world);
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
	const float relaxation_time = 0.5f;

	// add contact forces
	contact_forces_ = next_contact_forces_;

	// add and clamp the acceleration if SPH enabled
	if (world->GetIsActiveSPH())
	{
		// Use density-based blending if enabled
		if (world->GetIsActiveDensityBlending())
		{
			float kappa = (sph_density_ - 2.f) / (4.f - 2.f);
			if (sph_density_ < 2.f)
			{
				acceleration_ = next_acceleration_;
			}
			else
			{
				// Calculate a preferred goalReachingAcceleration based on GoalReachingForce since SPH alone does not take into account a global goal
				Vector2D goalReachingAcceleration = (getPreferredVelocity() - getVelocity()) / std::max(relaxation_time, getDeltaTime(world)) / getMass(); 
				//Vector2D goalReachingAcceleration = Vector2D(0, 0);
				if (sph_density_ > 4.f)
				{
					acceleration_ = goalReachingAcceleration + sph_acceleration_;
					//acceleration_ = sph_acceleration_;
				}
				else
				{
					acceleration_ = (1 - kappa) * (next_acceleration_) + kappa * (goalReachingAcceleration + sph_acceleration_);
					//acceleration_ = (1 - kappa) * (next_acceleration_) + kappa * (sph_acceleration_);
				}
			}
		}
		else
		{
			acceleration_ = next_acceleration_ + sph_acceleration_;
		}
	}
	else
	{
		acceleration_ = next_acceleration_;
	}

	// add contact forces to acceleration
	acceleration_ += contact_forces_ / settings_.mass_;

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

void Agent::UpdateVelocityAndPosition_RK4(WorldBase* world)
{
	const float dt = world->GetFineDeltaTime();
	const float relaxation_time = 0.5f;

	// add contact forces
	contact_forces_ = next_contact_forces_;
	original_velocity_ = velocity_;

	// add and clamp the acceleration if SPH enabled
	if (world->GetIsActiveSPH())
	{
		// Use density-based blending if enabled
		if (world->GetIsActiveDensityBlending())
		{
			float kappa = (sph_density_ - 2.f) / (4.f - 2.f);

			if (sph_density_ < 2.f)
			{
				k1_ = next_acceleration_;
				velocity_ = original_velocity_ + dt * k1_ / 2;
				ComputeAcceleration(world);
				k2_ = next_acceleration_;
				velocity_ = original_velocity_ + dt * k2_ / 2;
				ComputeAcceleration(world);
				k3_ = next_acceleration_;
				velocity_ = original_velocity_ + dt * k3_;
				ComputeAcceleration(world);
				k4_ = next_acceleration_;
			}
			else
			{
				// Calculate a preferred goalReachingAcceleration based on GoalReachingForce since SPH alone does not take into account a global goal
				//Vector2D goalReachingAcceleration = (getPreferredVelocity() - velocity_) / std::max(0.5f, getDeltaTime(world)) / getMass();
				if (sph_density_ > 4.f)
				{
					k1_ = (preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6;
					velocity_ = original_velocity_ + dt * k1_ / 2;
					k2_ = (preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6;
					velocity_ = original_velocity_ + dt * k2_ / 2;
					k3_ = (preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6;
					velocity_ = original_velocity_ + dt * k3_;
					k4_ = (preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6;
				}
				else
				{
					k1_ = (1 - kappa) * next_acceleration_ + kappa * ((preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
					velocity_ = original_velocity_ + dt * k1_ / 2;
					ComputeAcceleration(world);
					k2_ = (1 - kappa) * next_acceleration_ + kappa * ((preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
					velocity_ = original_velocity_ + dt * k2_ / 2;
					ComputeAcceleration(world);
					k3_ = (1 - kappa) * next_acceleration_ + kappa * ((preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
					velocity_ = original_velocity_ + dt * k3_;
					ComputeAcceleration(world);
					k4_ = (1 - kappa) * next_acceleration_ + kappa * ((preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
				}
			}
		}
		else
		{
			k1_ = next_acceleration_ + ((preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
			velocity_ = original_velocity_ + dt * k1_ / 2;
			ComputeAcceleration(world);
			k2_ = next_acceleration_ + ((preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
			velocity_ = original_velocity_ + dt * k2_ / 2;
			ComputeAcceleration(world);
			k3_ = next_acceleration_ + ((preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
			velocity_ = original_velocity_ + dt * k3_;
			ComputeAcceleration(world);
			k4_ = next_acceleration_ + ((preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
		}
	}
	else
	{
		k1_ = next_acceleration_;
		velocity_ = original_velocity_ + dt * k1_ / 2;
		ComputeAcceleration(world);
		k2_ = next_acceleration_;
		velocity_ = original_velocity_ + dt * k2_ / 2;
		ComputeAcceleration(world);
		k3_ = next_acceleration_;
		velocity_ = original_velocity_ + dt * k3_;
		ComputeAcceleration(world);
		k4_ = next_acceleration_;
	}
	acceleration_ = (k1_ + 2 * k2_ + 2 * k3_ + k4_) / 6;

	// add contact forces to acceleration
	acceleration_ += contact_forces_ / settings_.mass_;

	// integrate the velocity; clamp to a maximum speed
	velocity_ = clampVector(original_velocity_ + (acceleration_ * dt), getMaximumSpeed());

	// update the position
	position_ += velocity_ * dt;

	updateViewingDirection();
}

/*
void Agent::UpdateVelocityAndPosition_RK4(WorldBase* world)
{
	const float dt = world->GetFineDeltaTime();
	const float relaxation_time = 0.5f;

	// add contact forces
	contact_forces_ = next_contact_forces_;

	// add and clamp the acceleration if SPH enabled
	if (world->GetIsActiveSPH())
	{
		// Use density-based blending if enabled
		if (world->GetIsActiveDensityBlending())
		{
			float kappa = (sph_density_ - 2.f) / (4.f - 2.f);

			if (sph_density_ < 2.f)
			{
				k1_ = next_acceleration_;
				k2_ = ComputeAcceleration_RK4(velocity_ + dt * k1_ / 2, world);
				k3_ = ComputeAcceleration_RK4(velocity_ + dt * k2_ / 2, world);
				k4_ = ComputeAcceleration_RK4(velocity_ + dt * k3_, world);
				acceleration_ = (k1_ + 2 * k2_ + 2 * k3_ + k4_) / 6;
			}
			else
			{
				// Calculate a preferred goalReachingAcceleration based on GoalReachingForce since SPH alone does not take into account a global goal
				//Vector2D goalReachingAcceleration = (getPreferredVelocity() - velocity_) / std::max(0.5f, getDeltaTime(world)) / getMass();
				if (sph_density_ > 4.f)
				{
					k1_ = (preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6;
					k2_ = (preferred_velocity_ - (velocity_ + dt * k1_ / 2)) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6;
					k3_ = (preferred_velocity_ - (velocity_ + dt * k2_ / 2)) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6;
					k4_ = (preferred_velocity_ - (velocity_ + dt * k3_)) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6;
					acceleration_ = (k1_ + 2 * k2_ + 2 * k3_ + k4_) / 6;
				}
				else
				{
					k1_ = (1 - kappa) * (next_acceleration_) + kappa * ((preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
					k2_ = (1 - kappa) * (ComputeAcceleration_RK4(velocity_ + dt * k1_ / 2, world)) + kappa * ((preferred_velocity_ - (velocity_ + dt * k1_ / 2)) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
					k3_ = (1 - kappa) * (ComputeAcceleration_RK4(velocity_ + dt * k2_ / 2, world)) + kappa * ((preferred_velocity_ - (velocity_ + dt * k2_ / 2)) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
					k4_ = (1 - kappa) * (ComputeAcceleration_RK4(velocity_ + dt * k3_, world)) + kappa * ((preferred_velocity_ - (velocity_ + dt * k3_)) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
					acceleration_ = (k1_ + 2 * k2_ + 2 * k3_ + k4_) / 6;
				}
			}
		}
		else
		{
			k1_ = (next_acceleration_) + ((preferred_velocity_ - velocity_) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
			k2_ = (ComputeAcceleration_RK4(velocity_ + dt * k1_ / 2, world)) + ((preferred_velocity_ - (velocity_ + dt * k1_ / 2)) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
			k3_ = (ComputeAcceleration_RK4(velocity_ + dt * k2_ / 2, world)) + ((preferred_velocity_ - (velocity_ + dt * k2_ / 2)) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
			k4_ = (ComputeAcceleration_RK4(velocity_ + dt * k3_, world)) + ((preferred_velocity_ - (velocity_ + dt * k3_)) / std::max(relaxation_time, dt) / settings_.mass_ + sph_acceleration_ / 6);
			acceleration_ = (k1_ + 2 * k2_ + 2 * k3_ + k4_) / 6;
		}
	}
	else
	{
		k1_ = next_acceleration_;
		k2_ = ComputeAcceleration_RK4(velocity_ + dt * k1_ / 2, world);
		k3_ = ComputeAcceleration_RK4(velocity_ + dt * k2_ / 2, world);
		k4_ = ComputeAcceleration_RK4(velocity_ + dt * k3_, world);
		acceleration_ = (k1_ + 2 * k2_ + 2 * k3_ + k4_) / 6;
	}

	// add contact forces to acceleration
	acceleration_ += contact_forces_ / settings_.mass_;

	// integrate the velocity; clamp to a maximum speed
	velocity_ = clampVector(velocity_ + (acceleration_ * dt), getMaximumSpeed());

	// update the position
	position_ += velocity_ * dt;

	updateViewingDirection();
}
*/

void Agent::UpdateVelocityAndPosition_Verlet2(WorldBase* world)
{
	const float dt = world->GetFineDeltaTime();
	const float relaxation_time = 0.5f;

	// add contact forces
	contact_forces_ = next_contact_forces_;

	// add and clamp the acceleration if SPH enabled
	if (world->GetIsActiveSPH())
	{
		// Use density-based blending if enabled
		if (world->GetIsActiveDensityBlending())
		{
			float kappa = (sph_density_ - 2.f) / (4.f - 2.f);
			if (sph_density_ < 2.f)
			{
				acceleration_ = next_acceleration_;
			}
			else
			{
				// Calculate a preferred goalReachingAcceleration based on GoalReachingForce since SPH alone does not take into account a global goal
				Vector2D goalReachingAcceleration = (getPreferredVelocity() - getVelocity()) / std::max(relaxation_time, getDeltaTime(world)) / getMass();
				if (sph_density_ > 4.f)
				{
					acceleration_ = goalReachingAcceleration + sph_acceleration_;
				}
				else
				{
					acceleration_ = (1 - kappa) * (next_acceleration_) + kappa * (goalReachingAcceleration + sph_acceleration_);
				}
			}
		}
		else
		{
			acceleration_ = next_acceleration_ + sph_acceleration_;
		}
	}
	else
	{
		acceleration_ = next_acceleration_;
	}

	// add contact forces to acceleration
	acceleration_ += contact_forces_ / settings_.mass_;

	// integrate the velocity; clamp to a maximum speed
	velocity_ = clampVector(velocity_ + (acceleration_ * 0.5 * dt), getMaximumSpeed());

	// update the position
	position_ += velocity_ * dt;

	ComputeBaseSPH(world);
	ComputeDerivedSPH(world);
	ComputePreferredVelocity(world);
	ComputeAcceleration(world);
	ComputeContactForces(world);
	// add contact forces
	contact_forces_ = next_contact_forces_;

	// repeated to get updated acceleration
	if (world->GetIsActiveSPH())
	{
		// Use density-based blending if enabled
		if (world->GetIsActiveDensityBlending())
		{
			float kappa = (sph_density_ - 2.f) / (4.f - 2.f);
			if (sph_density_ < 2.f)
			{
				acceleration_ = next_acceleration_;
			}
			else
			{
				// Calculate a preferred goalReachingAcceleration based on GoalReachingForce since SPH alone does not take into account a global goal
				Vector2D goalReachingAcceleration = (getPreferredVelocity() - getVelocity()) / std::max(relaxation_time, getDeltaTime(world)) / getMass();
				if (sph_density_ > 4.f)
				{
					acceleration_ = goalReachingAcceleration + sph_acceleration_;
				}
				else
				{
					acceleration_ = (1 - kappa) * (next_acceleration_) + kappa * (goalReachingAcceleration + sph_acceleration_);
				}
			}
		}
		else
		{
			acceleration_ = next_acceleration_ + sph_acceleration_;
		}
	}
	else
	{
		acceleration_ = next_acceleration_;
	}

	// add contact forces to acceleration
	acceleration_ += contact_forces_ / settings_.mass_;
	velocity_ = clampVector(velocity_ + (acceleration_ * 0.5 * dt), getMaximumSpeed());

	updateViewingDirection();
}

void Agent::UpdateVelocityAndPosition_Leapfrog2(WorldBase* world)
{
	const float dt = world->GetFineDeltaTime();
	const float relaxation_time = 0.5f;

	// add contact forces
	contact_forces_ = next_contact_forces_;

	// add and clamp the acceleration if SPH enabled
	if (world->GetIsActiveSPH())
	{
		// Use density-based blending if enabled
		if (world->GetIsActiveDensityBlending())
		{
			float kappa = (sph_density_ - 2.f) / (4.f - 2.f);
			if (sph_density_ < 2.f)
			{
				acceleration_ = next_acceleration_;
			}
			else
			{
				// Calculate a preferred goalReachingAcceleration based on GoalReachingForce since SPH alone does not take into account a global goal
				Vector2D goalReachingAcceleration = (getPreferredVelocity() - getVelocity()) / std::max(relaxation_time, getDeltaTime(world)) / getMass();
				if (sph_density_ > 4.f)
				{
					acceleration_ = goalReachingAcceleration + sph_acceleration_;
				}
				else
				{
					acceleration_ = (1 - kappa) * (next_acceleration_) + kappa * (goalReachingAcceleration + sph_acceleration_);
				}
			}
		}
		else
		{
			acceleration_ = next_acceleration_ + sph_acceleration_;
		}
	}
	else
	{
		acceleration_ = next_acceleration_;
	}

	// add contact forces to acceleration
	acceleration_ += contact_forces_ / settings_.mass_;

	// integrate the velocity; clamp to a maximum speed
	velocity_ = clampVector(original_velocity_ + (acceleration_ * dt), getMaximumSpeed());
	if (world->GetCurrentTime() == 0.f)
		original_velocity_ = clampVector(original_velocity_ + (acceleration_ * 0.5 * dt), getMaximumSpeed());
	else
		original_velocity_ = velocity_;

	//velocity_ = clampVector(velocity_ + (acceleration_ * 0.5 * dt), getMaximumSpeed());
	//position_ += velocity_ * dt;
	//velocity_ = clampVector(velocity_ + (acceleration_ * 0.5 * dt), getMaximumSpeed());

	// update the position
	position_ += velocity_ * dt;

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

#pragma region [Advanced setters]

void Agent::setColorByDensity(float density)
{
	Color color;
	short r, g, b;
	float slider;
	float densityLimit = 10.f;
	float densityRatio = density / densityLimit;

	if (densityRatio < 0)
		densityRatio = 0;
	else if (densityRatio > 1)
		densityRatio = 1;
	
	if (densityRatio <= 0.1f) // Constant value for density beneath 1
	{
		r = 0;
		g = 0;
		b = 255;
	}
	else if (densityRatio <= 0.2f) // transition from blue(0, 0, 255) to lighter blue(0, 128, 255) // transition from green(0, 204, 102) to yellow(204, 204, 0)
	{
		slider = (densityRatio - 0.1f) / 0.1f;
		r = 0;
		g = (short)(128 * slider);
		b = 255;
	}
	else if (densityRatio <= 0.3f) // transition from lighter blue(0, 128, 255) to light blue(0, 255, 255)
	{
		slider = (densityRatio - 0.2f) / 0.1f;
		r = 0;
		g = 128 + (short)(127 * slider);
		b = 255;
	}
	else if (densityRatio <= 0.4f) // transition from light blue(0, 255, 255) to light green(0, 255, 128)
	{
		slider = (densityRatio - 0.3f) / 0.1f;
		r = 0;
		g = 255;
		b = 255 - (short)(127 * slider);
	}
	else if (densityRatio <= 0.5f) // transition from light green(0, 255, 128) to green(0, 255, 0)
	{
		slider = (densityRatio - 0.4f) / 0.1f;
		r = 0;
		g = 255;
		b = 128 - (short)(128 * slider);
	}
	else if (densityRatio <= 0.6f) // transition from green(0, 255, 0) to yellow(255, 255, 0)
	{
		slider = (densityRatio - 0.5f) / 0.1f;
		r = (short)(255 * slider);
		g = 255;
		b = 0;
	}
	else if (densityRatio <= 0.7f) // transition from yellow(255, 255, 0) to orange(255, 128, 0)
	{
		slider = (densityRatio - 0.6f) / 0.1f;
		r = 255;
		g = 255 - (short)(127 * slider);
		b = 0;
	}
	else if (densityRatio <= 0.8f) // transition from orange(255, 128, 0) to red(255, 0, 0)
	{
		slider = (densityRatio - 0.7f) / 0.1f;
		r = 255;
		g = 128 - (short)(128 * slider);
		b = 0;
	}
	else if (densityRatio <= 0.9f) // transition from red(255, 0, 0) to dark red(128, 0, 0)
	{
		slider = (densityRatio - 0.8f) / 0.1f;
		r = 255 - (short)(127 * slider);
		g = 0;
		b = 0;
	}
	else if (densityRatio <= 1.0f) // transition from dark red(128, 0, 0) to brown(64, 0, 0)
	{
		slider = (densityRatio - 0.9f) / 0.1f;
		r = 128 - (short)(64 * slider);
		g = 0;
		b = 0;
	}
	else // 
	{
		r = 64;
		g = 0;
		b = 0;
	}
	color = Color(r, g, b);
	setColor(color);
}

#pragma endregion

float Agent::ComputeRandomNumber(float min, float max)
{
	auto distribution = std::uniform_real_distribution<float>(min, max);
	return distribution(RNGengine_);
}