#include <core/sph.h>

#define PI 3.14159265f

#pragma region [Smoothing-kernel methods]

// Might want to consider refactoring h as a parameter
float SPH::poly6_kernel(Vector2D pos)
{
	float magnitude_sq = pow(pos.x, 2) + pow(pos.y, 2);
	float magnitude = sqrt(magnitude_sq);
	if (magnitude < 1)
		return (4 * pow((1 - magnitude_sq), 3)) / PI;
	else
		return 0;
}

Vector2D SPH::spiky_kernel(Vector2D pos)
{
	float magnitude = sqrt(pow(pos.x, 2) + pow(pos.y, 2));
	Vector2D normalized_pos = pos / magnitude;
	if (magnitude < 1)
		return (-30 * normalized_pos * pow(1 - magnitude, 2)) / PI;
	else
		return Vector2D(0, 0);
}

float SPH::mullers_kernel(Vector2D pos)
{
	float magnitude = sqrt(pow(pos.x, 2) + pow(pos.y, 2));
	if (magnitude < 1)
		return (360 * (1 - magnitude)) / (29 * PI);
	else
		return 0;
}

#pragma endregion

float SPH::clampPersonalRestDensity(float rest_density)
{
	if (rest_density < settings_.min_rest_density)
		return settings_.min_rest_density;
	else if (rest_density > settings_.max_rest_density)
		return settings_.max_rest_density;
	else
		return rest_density;
}