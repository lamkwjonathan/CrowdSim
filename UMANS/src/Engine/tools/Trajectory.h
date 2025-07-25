/* UMANS: Unified Microscopic Agent Navigation Simulator
** MIT License
** Copyright (C) 2018-2020  Inria Rennes Bretagne Atlantique - Rainbow - Julien Pettr�
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

#ifndef LIB_TRAJECTORY_H
#define LIB_TRAJECTORY_H

#include <tools/vector2D.h>
#include <map>
#include <tools/Color.h>

///<summary>A struct describing a position and orientation at a particular time.</summary>
struct TrajectoryPoint
{
	double time;
	Vector2D position;
	Vector2D orientation;
	Color color;

	TrajectoryPoint() {}
	TrajectoryPoint(double time, const Vector2D& position, const Vector2D& orientation, const Color color)
		: time(time), position(position), orientation(orientation), color(color) {}
};

typedef std::vector<TrajectoryPoint> Trajectory;

typedef std::map<size_t, TrajectoryPoint> AgentTrajectoryPoints;
typedef std::map<size_t, Trajectory> AgentTrajectories;

#endif // LIB_TRAJECTORY_H