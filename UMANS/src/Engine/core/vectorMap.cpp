#include "vectorMap.h"
#include "omp.h""

vectorMap::vectorMap(int width, int height, int num_threads) 
{
	width_ = width;
	height_ = height;
	vectorArray_ = std::make_unique<Vector2D[]>(width * height);
	distanceArray_ = std::make_unique<float[]>(width * height);
	weightedCountArray_ = std::unique_ptr<float[]>(new float[num_threads]()); // automatically initializes to zero
	congestionValueArray_ = std::unique_ptr<float[]>(new float[num_threads]()); // automatically initializes to zero
	agentCountArray_ = std::unique_ptr<int[]>(new int[num_threads]()); // automatically initializes to zero
	speedValueArray_ = std::unique_ptr<float[]>(new float[num_threads]()); // automatically initializes to zero
}

void vectorMap::collateThreadLocalVariables()
{
	for (int i = 0; i < omp_get_max_threads(); ++i)
	{
		weightedCount_ += weightedCountArray_[i];
		weightedCountArray_[i] = 0.0f;

		congestionValue_ += congestionValueArray_[i];
		congestionValueArray_[i] = 0.0f;

		agentCount_ += agentCountArray_[i];
		agentCountArray_[i] = 0.0f;

		speedValue_ += speedValueArray_[i];
		speedValueArray_[i] = 0.0f;
	}
}

//float vectorMap::multiplierFromCongestionValue(float congestionValue)
//{
//	if (congestionValue <= 2.0f)
//	{
//		return 1.0f;
//	}
//	else if (congestionValue <= 3.0f)
//	{
//		return 1.0f + (congestionValue - 2.0f) * 0.75f;
//	}
//	else if (congestionValue <= 4.0f)
//	{
//		return 1.75f + (congestionValue - 3.0f) * 0.55f;
//	}
//	else if (congestionValue <= 5.0f)
//	{
//		return 2.3f + (congestionValue - 4.0f) * 0.5f;
//	}
//	else if (congestionValue <= 6.0f)
//	{
//		return 2.8f + (congestionValue - 5.0f) * 0.4f;
//	}
//	else if (congestionValue <= 7.0f)
//	{
//		return 3.2f + (congestionValue - 6.0f) * 0.3f;
//	}
//	else
//	{
//		return 3.5f;
//	}
//}

float vectorMap::multiplierFromCongestionValue(float congestionValue)
{
	if (congestionValue <= 1.0f)
	{
		return 1.0f;
	}
	else if (congestionValue <= 2.0f)
	{
		return 1.0f + (congestionValue - 1.0f) * 0.75f;
	}
	else if (congestionValue <= 3.0f)
	{
		return 1.75f + (congestionValue - 2.0f) * 0.55f;
	}
	else if (congestionValue <= 4.0f)
	{
		return 2.3f + (congestionValue - 3.0f) * 0.5f;
	}
	else if (congestionValue <= 5.0f)
	{
		return 2.8f + (congestionValue - 4.0f) * 0.4f;
	}
	else if (congestionValue <= 6.0f)
	{
		return 3.2f + (congestionValue - 5.0f) * 0.3f;
	}
	else
	{
		return 7.0f;
	}
}

float vectorMap::speedFromSPHDensity(float density)
{
	if (density <= 1.0f)
		return 1.4f;
	else if (density <= 2.0f)
		return 1.4f - (density - 1.0f) * 0.6f;
	else if (density <= 3.0f)
		return 0.8f - (density - 2.0f) * 0.2f;
	else if (density <= 4.0f)
		return 0.6f - (density - 3.0f) * 0.1f;
	else if (density <= 5.0f)
		return 0.5f - (density - 4.0f) * 0.1f;
	else if (density <= 6.0f)
		return 0.4f - (density - 5.0f) * 0.05f;
	else
		return 0.2f;
}


Vector2D vectorMap::preferredVelocityFromSPHDensity(Vector2D velocity, float density)
{
	float naturalWalkingSpeed = std::max(0.0, pow(density/0.24/1.72 * 0.3, 2));
	return Vector2D(velocity.x * naturalWalkingSpeed, velocity.y * naturalWalkingSpeed) / 1.4f;
}