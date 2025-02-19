#include <core/sph.h>

#define PI 3.14159265f

#pragma region [Smoothing-kernel methods]

// Might want to consider refactoring h as a parameter
float SPH::poly6_kernel(Vector2D pos)
{
	float magnitude_sq = pos.sqrMagnitude();
	float magnitude = sqrt(magnitude_sq);
	if (magnitude < 1)
		return (4 * pow((1 - magnitude_sq), 3)) / PI;
	else
		return 0;
}

Vector2D SPH::spiky_kernel(Vector2D pos)
{
	float magnitude = pos.magnitude();
	Vector2D normalized_pos = pos / magnitude;
	if (magnitude < 1)
		return (-30 * normalized_pos * pow(1 - magnitude, 2)) / PI;
	else
		return Vector2D(0, 0);
}

float SPH::mullers_kernel(Vector2D pos)
{
	float magnitude = pos.magnitude();
	if (magnitude < 1)
		return (360 * (1 - magnitude)) / (29 * PI);
	else
		return 0;
}

#pragma endregion

Vector2D SPH::calcRepresentativePoint(Vector2D pos, Vector2D nearest_point)
{
	Vector2D furthest_point = (nearest_point - pos).getnormalized();
	return (furthest_point + nearest_point) / 2;
}

float SPH::calcObstacleArea(Vector2D pos, LineSegment2D line)
{
	// Compute circle-line intersection
	// Circle: (x - pos.x)^2 + (y - pos.y)^2 = r^2
	// Line: x = x1 + t(x2 - x1); y = y1 + t(y2 - y1);
	float A, B, C;
	float dx, dy;
	float X, Y;
	float det;
	dx = line.second.x - line.first.x;
	dy = line.second.y - line.first.y;
	X = line.first.x - pos.x;
	Y = line.first.y - pos.y;
	A = pow(dx, 2) + pow(dy, 2);
	B = 2 * (X * dx + Y * dy);
	C = X * X + Y * Y - 1;
	det = pow(B, 2) - 4 * A * C;
	if (det <= 0) // If no intersection or one intersection (tangent), hidden area always equals zero.
		return 0.f;
	else
	{
		Vector2D endPoint1, endPoint2;
		float ang;
		float area;
		float t1, t2;
		t1 = (-B - det) / (2 * A);
		t2 = (-B + det) / (2 * A);
		
		if ((line.first - pos).magnitude() < 1) 
		{
			endPoint1 = line.first; // point is within 1 meter radius, therefore point itself is the endpoint
		}
		else
		{
			endPoint1 = Vector2D(line.first.x + t1 * dx, line.first.y + t1 * dy); // point is the intersection between the circle of 1 meter radius centered at the agent position and the line obstacle
		}
			
		if ((line.second - pos).magnitude() <= 1) 
		{
			endPoint2 = line.second; // point is within 1 meter radius, therefore point itself is the endpoint
		}
		else
		{
			endPoint2 = Vector2D(line.first.x + t2 * dx, line.first.y + t2 * dy); // point is the intersection between the circle of 1 meter radius centered at the agent position and the line obstacle
		}
		
		ang = angle(endPoint1 - pos, endPoint2 - pos);
		area = ang / 2 - 0.5 * (pos.x*(endPoint1.y - endPoint2.y) + endPoint1.x*(endPoint2.y - pos.y) + endPoint2.x*(pos.y-endPoint1.y)); // area = area of sector of circle - area of trangle in front of wall (calculated using shoelace method);
		
		if (area < 0) // sometimes marginally negative values appear due to rounding error
			return 0.f; // set back to zero
		else
			return area;
	}
}

float SPH::clampPersonalRestDensity(float rest_density)
{
	if (rest_density < settings_.min_rest_density)
		return settings_.min_rest_density;
	else if (rest_density > settings_.max_rest_density)
		return settings_.max_rest_density;
	else
		return rest_density;
}