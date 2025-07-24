#ifndef LIB_VMAP_H
#define LIB_VMAP_H

#include <iostream>
#include <tools/Polygon2D.h>
#include <mutex>

class vectorMap

{
protected:
	/// <summary>A vector array containing the global navigation direction vector for each grid square.</summary>
	std::unique_ptr<Vector2D[]> vectorArray_;

	/// <summary>A vector array containing the distance from each grid square to nearest goal.</summary>
	std::unique_ptr<float[]> distanceArray_;

	/// <summary>A Vector2D representing the goal position of this map.</summary>
	Vector2D goal_ = Vector2D(0, 0);

	/// <summary>A float value keeping track of the accumulated agent count weighted according to heading relative to goal.</summary>
	float weightedCount_ = 1.0f;

	/// <summary>An array storing thread-local values of weightedCount_.</summary>
	std::unique_ptr<float[]> weightedCountArray_;

	/// <summary>A float value keeping track of the accumulated congestion per time step.</summary>
	float congestionValue_ = 1.0f;

	/// <summary>An array storing thread-local values of congestionValue_.</summary>
	std::unique_ptr<float[]> congestionValueArray_;

	/// <summary>An int value keeping track of the number of agents heading towards this goal.</summary>
	int agentCount_ = 1;

	/// <summary>An array storing thread-local values of agentCount_.</summary>
	std::unique_ptr<int[]> agentCountArray_;

	/// <summary>A float value keeping track of the accumulated speed of agents moving towards this goal per time step.</summary>
	float speedValue_ = 1.4f;

	/// <summary>An array storing thread-local values of speedValue_.</summary>
	std::unique_ptr<float[]> speedValueArray_;

	/// <summary>A float value keeping track of the moving average distance multiplier penalty of this map.</summary>
	float distanceMultiplier_ = 1.0f;

	/// <summary>A int value indicating the world width.</summary>
	int width_ = 0;

	/// <summary>A int value indicating the world height.</summary>
	int height_ = 0;

public:
#pragma region [Basic getters]
	/// @name Basic getters
	/// Methods that directly return a value stored in the instance of the vectorMap class.
	/// @{

	/// <summary>Returns the corresponding global navigation vector for an agent's current position.</summary>
	inline Vector2D getVector(float x, float y) const { return vectorArray_[(int)std::floor(x) + (int)std::floor(y) * width_]; }
	/// <summary>Returns the corresponding distance value for an agent's current position.</summary>
	inline float getDistance(float x, float y) const { return distanceArray_[(int)std::floor(x) + (int)std::floor(y) * width_]; }
	/// <summary>Returns the current vectorMap goal point.</summary>
	inline Vector2D getGoal() const { return goal_; }
	/// <summary>Returns the accumulated agent count weighted according to heading relative to goal.</summary>
	inline float getWeightedCount() const { return weightedCount_; }
	/// <summary>Returns the current vectorMap congestion value.</summary>
	inline float getCongestionValue() const { return congestionValue_; }
	/// <summary>Returns the current vectorMap agent count.</summary>
	inline int getAgentCount() const { return agentCount_; }
	/// <summary>Returns the current vectorMap speed value.</summary>
	inline float getSpeedValue() const { return speedValue_; }
	/// <summary>Returns the current vectorMap distance multiplier penalty.</summary>
	inline float getDistanceMultiplier() const { return distanceMultiplier_; }

	/// @}
#pragma endregion

#pragma region [Basic setters]
	/// @name Basic setters
	/// Methods that directly change a value stored in the instance of the SPH class.
	/// @{

	/// <summary>Sets the vectorMap global navigation vector for an agent's current position.</summary>
	/// <param name="direction">Vector2D representing the new normalized global navigation vector.</param>
	inline void setVector(int x, int y, Vector2D direction) { vectorArray_[x + y * width_] = direction; }

	/// <summary>Sets the vectorMap global navigation vector for an agent's current position.</summary>
	/// <param name="direction">Vector2D representing the new normalized global navigation vector.</param>
	inline void setDistance(int x, int y, float distance) { distanceArray_[x + y * width_] = distance; }

	/// <summary>Sets the vectorMap goal point to the given vector.</summary>
	/// <param name="c">Vector2D representing the new goal point.</param>
	inline void setGoal(Vector2D goal) { goal_ = goal; }

	/// <summary>Sets the accumulated weighted agent count to the given value.</summary>
	/// <param name="count">Float representing the new weighted agent count.</param>
	inline void setWeightedCount(float count) { weightedCount_ = count; }

	/// <summary>Sets the vectorMap congestion value to the given value.</summary>
	/// <param name="c">Float representing the new congestion value.</param>
	inline void setCongestionValue(float c) { congestionValue_ = c; }

	/// <summary>Sets the agent count to the given value.</summary>
	/// <param name="count">Float representing the new agent count.</param>
	inline void setAgentCount(int count) { agentCount_ = count; }

	/// <summary>Sets the vectorMap speed value to the given value.</summary>
	/// <param name="s">Float representing the new speed value.</param>
	inline void setSpeedValue(float s) { speedValue_ = s; }

	/// <summary>Sets the vectorMap distance multiplier penalty to the given value.</summary>
	/// <param name="d">Float representing the new distance multiplier penalty.</param>
	inline void setDistanceMultiplier(float d) { distanceMultiplier_ = d; }

	/// @}
#pragma endregion

	/// <summary>Increments the thread-local accumulated weighted agent count by the given value.</summary>
	/// <param name="count">Float representing the increment value.</param>
	/// <param name="thread_id">Int representing the thread ID.</param>
	inline void incrementWeightedCount(float count, int thread_id) { weightedCountArray_[thread_id] += count; }

	/// <summary>Increments the thread-local vectorMap congestion value by the given value.</summary>
	/// <param name="c">Float representing the increment value.</param>
	/// <param name="thread_id">Int representing the thread ID.</param>
	inline void incrementCongestionValue(float c, int thread_id) { congestionValueArray_[thread_id] += c; }

	/// <summary>Increments the thread-local agent count by the given value.</summary>
	/// <param name="count">Float representing the increment value.</param>
	/// <param name="thread_id">Int representing the thread ID.</param>
	inline void incrementAgentCount(int count, int thread_id) { agentCountArray_[thread_id] += count; }

	/// <summary>Increments the thread-local vectorMap speed value by the given value.</summary>
	/// <param name="speed">Float representing the increment value.</param>
	/// <param name="thread_id">Int representing the thread ID.</param>
	inline void incrementSpeedValue(float speed, int thread_id) { speedValueArray_[thread_id] += speed; }

	/// <summary>Collates the thread-local arrays into their respective variables.</summary>
	void collateThreadLocalVariables();

	/// <summary>Creates a vectorMap object.</summary>
	vectorMap(int width, int height, int num_threads);

	/// <summary>Converts the vectorMap congestion value into a distance multiplier based on speed.</summary>
	/// <param name="congestionValue">Float representing the congestionValue of the vectorMap.</param>
	/// <returns>A float value representing the multiplier value.</returns>
	static float multiplierFromCongestionValue(float congestionValue);

	/// <summary>Converts the SPH density into the corresponding preferred speed.</summary>
	/// <param name="density">Float representing the SPH density of the agent.</param>
	/// <returns>A float value representing the new preferred speed.</returns>
	static float speedFromSPHDensity(float density);

	/// <summary>Takes original preferred velocity and SPH density and returns the corresponding preferred velocity.</summary>
	/// <param name="velocity">Vector2D representing the original preferred velocity of the agent.</param>
	/// <param name="density">Float representing the SPH density of the agent.</param>
	/// <returns>A Vector2D representing the new preferred velocity.</returns>
	static Vector2D preferredVelocityFromSPHDensity(Vector2D velocity, float density);
};

#endif //LIB_VMAP_H