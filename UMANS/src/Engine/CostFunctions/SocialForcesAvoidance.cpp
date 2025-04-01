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

#include <CostFunctions/SocialForcesAvoidance.h>
#include <core/agent.h>
#include <core/worldBase.h>

Vector2D SocialForcesAvoidance::ComputeAgentInteractionForce(const Agent* agent, const PhantomAgent& other) const
{
	// This is an implementation of equations from the 1995 paper by Helbing and Molnar:
	//
	// R = agent.position - other.position,
	// F = -nabla_R V(b(R)), {Eq. 3}
	// where V is a scalar potential function: 
	// V(b(R)) = V0 * e^(-b(R) / sigma), {Eq. 13}
	// where b is the axis of an ellipse: 
	// b(R) = 0.5 * sqrt[(||R|| + ||R - vb*dt*Eb||)^2 - (vb*dt)^2]. {Eq. 4}
	//
	// Main derivative:
	// F = - d/dR [ V0 * e^(-b(R) / sigma) ]
	//   = - V0 * -1 / sigma * e^(-b(R) / sigma) * d/dR b(R)
	//   = V0/sigma * e^(-b(R) / sigma) * d/dR b(R)

	const Vector2D& R = agent->getPosition() - other.GetPosition();
	float magR = R.magnitude();

	// if the agents are already colliding right now, don't apply any force; we'll do this via contact forces instead
	// --> Disabled for now because it makes the model non-smooth.
	//if (magR <= agent->getRadius() + other.realAgent->getRadius())
	//	return Vector2D(0, 0);

	// velocity of the other agent
	// --- 1995 version: use current and preferred velocity of B
	//float vb = other.GetVelocity().magnitude();
	//const Vector2D& Eb = other.realAgent->getPreferredVelocity().getnormalized();
	//float magV = vb * dt;
	//const Vector2D& V = Eb * magV;
	// --- 2013 version: use the relative velocity
	const Vector2D& Vb = other.GetVelocity() - agent->getVelocity(); 
	const Vector2D& V = Vb * dt;
	float magV = V.magnitude();

	// B = semi-minor axis of an ellipse, describing the repulsive potential,
	// defined in Eq. 4 of the paper as: b(R) = 0.5 * sqrt[(||R|| + ||R - vb*dt*Eb||)^2 - (vb*dt)^2],
	// so (2b)^2 = (||R|| + ||R - vb*dt*Eb||)^2 - (vb*dt)^2
	
	const Vector2D& RminV = R - V;
	float magRminV = RminV.magnitude();
	float bSquared2 = pow(magR + magRminV, 2) - pow(magV, 2);
	if (bSquared2 <= 0)
		return Vector2D(0, 0);

	float b = 0.5f*sqrtf(bSquared2);

	// ignore very small values of b
	// --> Disabled for now because it seems unnecessary, and it is not mentioned in any papers.
	//if (b < 0.001f)
	//	return Vector2D(0, 0);

	// The closed form of d/dR b(R) is never reported in literature, so perhaps it is considered trivial.
	// Because we do not agree with that, here is our expansion: 
	//
	// d/dR b(R) = d/dR [ 1/2 * sqrt[(||R|| + ||R - V||)^2 - (vb*dt)^2] ]
	//           = d/dR [ 1/2 * sqrt(bSquared2(R)) ]                 { def. of bSquared2 as computed above }
	//           = 1/(4*sqrt(bSquared2(R))) * d/dR bSquared2(R)      { chain rule }
	//           = 1/(4*2b(R)) * d/dR bSquared2(R)                   { because sqrt(bSquared2(R)) = 2*b(R) }
	//           = 1/(8b(R))   * d/dR bSquared2(R)
	//
	// where
	// d/dR bSquared2(R) = 2 * (||R|| + ||R - V||) * [ d/dR ||R|| + d/dR ||R - V|| ]    { chain rule }
	//                   = 2 * (||R|| + ||R - V||) * [ R / ||R|| + (R - V) / ||R - V|| ]
	//
	// So in total:
	//
	// F = V0 / sigma * e^(-b(R)/sigma) * (||R|| + ||R-V||) / (4b(R)) * [ R / ||R|| + (R - V) / ||R - V|| ]

	const Vector2D& Force = V0 / sigma * expf(-b/sigma) * (magR + magRminV) / (4 * b) * (R / magR + RminV / magRminV);

	// scale the force down if the other agent is outside our view
	float Scale = (angle(agent->getVelocity(), -R) < ViewingAngleHalf ? 1 : ScaleOutsideView);
	return Scale * Force;
}

Vector2D SocialForcesAvoidance::ComputeAgentInteractionForce_RK4(const Agent* agent, const Vector2D velocity, const PhantomAgent& other) const
{
	// This is an implementation of equations from the 1995 paper by Helbing and Molnar:
	//
	// R = agent.position - other.position,
	// F = -nabla_R V(b(R)), {Eq. 3}
	// where V is a scalar potential function: 
	// V(b(R)) = V0 * e^(-b(R) / sigma), {Eq. 13}
	// where b is the axis of an ellipse: 
	// b(R) = 0.5 * sqrt[(||R|| + ||R - vb*dt*Eb||)^2 - (vb*dt)^2]. {Eq. 4}
	//
	// Main derivative:
	// F = - d/dR [ V0 * e^(-b(R) / sigma) ]
	//   = - V0 * -1 / sigma * e^(-b(R) / sigma) * d/dR b(R)
	//   = V0/sigma * e^(-b(R) / sigma) * d/dR b(R)

	const Vector2D& R = agent->getPosition() - other.GetPosition();
	float magR = R.magnitude();

	// if the agents are already colliding right now, don't apply any force; we'll do this via contact forces instead
	// --> Disabled for now because it makes the model non-smooth.
	//if (magR <= agent->getRadius() + other.realAgent->getRadius())
	//	return Vector2D(0, 0);

	// velocity of the other agent
	// --- 1995 version: use current and preferred velocity of B
	//float vb = other.GetVelocity().magnitude();
	//const Vector2D& Eb = other.realAgent->getPreferredVelocity().getnormalized();
	//float magV = vb * dt;
	//const Vector2D& V = Eb * magV;
	// --- 2013 version: use the relative velocity
	const Vector2D& Vb = other.GetVelocity() - agent->getVelocity();
	const Vector2D& V = Vb * dt;
	float magV = V.magnitude();

	// B = semi-minor axis of an ellipse, describing the repulsive potential,
	// defined in Eq. 4 of the paper as: b(R) = 0.5 * sqrt[(||R|| + ||R - vb*dt*Eb||)^2 - (vb*dt)^2],
	// so (2b)^2 = (||R|| + ||R - vb*dt*Eb||)^2 - (vb*dt)^2

	const Vector2D& RminV = R - V;
	float magRminV = RminV.magnitude();
	float bSquared2 = pow(magR + magRminV, 2) - pow(magV, 2);
	if (bSquared2 <= 0)
		return Vector2D(0, 0);

	float b = 0.5f * sqrtf(bSquared2);

	// ignore very small values of b
	// --> Disabled for now because it seems unnecessary, and it is not mentioned in any papers.
	//if (b < 0.001f)
	//	return Vector2D(0, 0);

	// The closed form of d/dR b(R) is never reported in literature, so perhaps it is considered trivial.
	// Because we do not agree with that, here is our expansion: 
	//
	// d/dR b(R) = d/dR [ 1/2 * sqrt[(||R|| + ||R - V||)^2 - (vb*dt)^2] ]
	//           = d/dR [ 1/2 * sqrt(bSquared2(R)) ]                 { def. of bSquared2 as computed above }
	//           = 1/(4*sqrt(bSquared2(R))) * d/dR bSquared2(R)      { chain rule }
	//           = 1/(4*2b(R)) * d/dR bSquared2(R)                   { because sqrt(bSquared2(R)) = 2*b(R) }
	//           = 1/(8b(R))   * d/dR bSquared2(R)
	//
	// where
	// d/dR bSquared2(R) = 2 * (||R|| + ||R - V||) * [ d/dR ||R|| + d/dR ||R - V|| ]    { chain rule }
	//                   = 2 * (||R|| + ||R - V||) * [ R / ||R|| + (R - V) / ||R - V|| ]
	//
	// So in total:
	//
	// F = V0 / sigma * e^(-b(R)/sigma) * (||R|| + ||R-V||) / (4b(R)) * [ R / ||R|| + (R - V) / ||R - V|| ]

	const Vector2D& Force = V0 / sigma * expf(-b / sigma) * (magR + magRminV) / (4 * b) * (R / magR + RminV / magRminV);

	// scale the force down if the other agent is outside our view
	float Scale = (angle(agent->getVelocity(), -R) < ViewingAngleHalf ? 1 : ScaleOutsideView);
	return Scale * Force;
}

Vector2D SocialForcesAvoidance::ComputeObstacleInteractionForce(const Agent* agent, const LineSegment2D& obstacle) const
{
	// find the nearest point on the obstacle
	const Vector2D& nearest = nearestPointOnLine(agent->getPosition(), obstacle.first, obstacle.second, true);
	const Vector2D& diff = agent->getPosition() - nearest;

	// if the distance is extremely small, ignore it
	float dist = diff.magnitude();
	if (dist < 0.001f)
		return Vector2D(0, 0);

	// F = -nabla U(R), { Eq. 5 in the paper }
	// where U(R) is a scalar potential function:
	// U(R) = U0 * e^(-||R|| / sigma). { Eq. 13 }
	// 
	// -d/dR U(R) = -1* -U0/sigma * e^(-||R|| / sigma) * d/dR ||R||
	//           = U0/sigma * e^(-||R|| / sigma) * R / ||R||.
	//           = R * U0 * e^(-||R|| / sigma) / (sigma * ||R||) 
	//
	// (Watch out: the 'sigma' here is actually called 'R' in the paper and in the code, 
	// but our explanation uses naming that matches the one from ComputeAgentInteractionForce().)
	
	const Vector2D& Force = diff * (U0 * exp(-dist / R) / (R * dist));

	// scale the force down if the other agent is outside our view
	float Scale = (angle(agent->getVelocity(), -diff) < ViewingAngleHalf ? 1 : ScaleOutsideView);
	return Scale * Force;
}

Vector2D SocialForcesAvoidance::ComputeObstacleInteractionForce_RK4(const Agent* agent, const Vector2D velocity, const LineSegment2D& obstacle) const
{
	// find the nearest point on the obstacle
	const Vector2D& nearest = nearestPointOnLine(agent->getPosition(), obstacle.first, obstacle.second, true);
	const Vector2D& diff = agent->getPosition() - nearest;

	// if the distance is extremely small, ignore it
	float dist = diff.magnitude();
	if (dist < 0.001f)
		return Vector2D(0, 0);

	// F = -nabla U(R), { Eq. 5 in the paper }
	// where U(R) is a scalar potential function:
	// U(R) = U0 * e^(-||R|| / sigma). { Eq. 13 }
	// 
	// -d/dR U(R) = -1* -U0/sigma * e^(-||R|| / sigma) * d/dR ||R||
	//           = U0/sigma * e^(-||R|| / sigma) * R / ||R||.
	//           = R * U0 * e^(-||R|| / sigma) / (sigma * ||R||) 
	//
	// (Watch out: the 'sigma' here is actually called 'R' in the paper and in the code, 
	// but our explanation uses naming that matches the one from ComputeAgentInteractionForce().)

	const Vector2D& Force = diff * (U0 * exp(-dist / R) / (R * dist));

	// scale the force down if the other agent is outside our view
	float Scale = (angle(agent->getVelocity(), -diff) < ViewingAngleHalf ? 1 : ScaleOutsideView);
	return Scale * Force;
}

void SocialForcesAvoidance::parseParameters(const CostFunctionParameters & params)
{
	ForceBasedFunction::parseParameters(params);
	params.ReadFloat("dt", dt);
	params.ReadFloat("V0", V0);
	params.ReadFloat("sigma", sigma);
	params.ReadFloat("U0", V0);
	params.ReadFloat("R", R);
}
