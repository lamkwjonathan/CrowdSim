#ifndef LIB_SPH_H
#define LIB_SPH_H

#include <tools/vector2D.h>

class WorldBase;

/// <summary>Class containing SPH parameters and functions.</summary>
class SPH
{
public:
	/// <summary>A struct containing the SPH settings that typically do not change after initialization.</summary>
	struct Settings
	{
		/// <summary>SPH gas constant that is fixed to a default value for this particular crowd simulation set-up.</summary>
		float gas_constant = 200.0f;

		/// <summary>SPH viscosity constant that is fixed to a default value for this particular crowd simulation set-up.</summary>
		float viscosity_constant = 0.0f;

		/// <summary>User-controllable SPH maximum rest density that affects highest allowable crowd density in the simulation.</summary>
		float max_rest_density = 5.0f;

		/// <summary>SPH minimum rest density that is fixed to 0 for this particular crowd simulation set-up.</summary>
		float min_rest_density = 0.0f;

		/// <summary>SPH minimum rest density that is fixed to a default value for this particular crowd simulation set-up.</summary>
		float density_time_window = 0.1f;
	};

private:
	SPH::Settings settings_;

	// Private constructor; only the world should create SPH instances.
	//SPH();
	//friend WorldBase;

public:
#pragma region [Smoothing-kernel methods]
	/// @name Smoothing-kernel methods
	/// Public methods that are used in density, pressure and viscosity calculations
	/// @{

	/// <summary>Poly6 smoothing-kernel used in density calculations.</summary>
	/// <remarks></remarks>
	/// <param name="pos">Vector2D denoting worldspace position of agent in question.</param>
	float poly6_kernel(Vector2D pos);

	/// <summary>Gradient of spiky smoothing-kernel used in pressure calculations.</summary>
	/// <remarks></remarks>
	/// <param name="pos">Vector2D denoting worldspace position of agent in question.</param>
	Vector2D spiky_kernel(Vector2D pos);

	/// <summary>2nd derivative of Muller's smoothing-kernel used in viscosity calculations.</summary>
	/// <remarks></remarks>
	/// <param name="pos">Vector2D denoting worldspace position of agent in question.</param>
	float mullers_kernel(Vector2D pos);

	/// @}
#pragma endregion

#pragma region [Basic getters]
	/// @name Basic getters
	/// Methods that directly return a value stored in the instance of the SPH class.
	/// @{

	/// <summary>Returns the SPH Gas Constant.</summary>
	inline float getGasConstant() const { return settings_.gas_constant; }
	/// <summary>Returns the SPH Viscosity Constant.</summary>
	inline float getViscosityConstant() const { return settings_.viscosity_constant; }
	/// <summary>Returns the SPH Maximum Rest Density.</summary>
	inline float getMaxRestDensity() const { return settings_.max_rest_density; }
	/// <summary>Returns the SPH Mimimum Rest Density.</summary>
	inline float getMinRestDensity() const { return settings_.min_rest_density; }
	/// <summary>Returns the time window for calculation of change in personal rest density over time.</summary>
	inline float getDensityTimeWindow() const { return settings_.density_time_window; }

	/// @}
#pragma endregion

#pragma region [Basic setters]
	/// @name Basic setters
	/// Methods that directly change a value stored in the instance of the SPH class.
	/// @{

	/// <summary>Sets the maximum rest density to the given value.</summary>
	/// <param name="max_density">Float representing the new maximum rest density.</param>
	inline void setMaxRestDensity(const float max_density) { settings_.max_rest_density = max_density; }

	/// @}
#pragma endregion

	/// <summary>Method that calculates and returns the representative point of a given line segment obstacle with respect to an agent's position.</summary>
	/// <param name="pos">The position of agent in question.</param>
	/// <param name="nearest_point">The position of the nearest point on the line segment obstacle in question.</param>
	/// <returns>The representative point of the given line segment.</returns>
	Vector2D calcRepresentativePoint(Vector2D pos, Vector2D nearest_point);

	/// <summary>Method that calculates and returns surface area of line segment obstacle within 1 meter of agent.</summary>
	/// <param name="pos">The position of agent in question.</param>
	/// <param name="line">The line segment obstacle in question.</param>
	/// <returns>Surface area of line segment obstacle within 1 meter of agent.</returns>
	float calcObstacleArea(Vector2D pos, LineSegment2D line);

	/// <summary>Method that clamps personal rest density between min_rest_density and max_rest_density</summary>
	/// <param name="rest_density">The pre-clamped rest density calculated for an agent.</param>
	/// <returns>Clamped personal rest density</returns>
	float clampPersonalRestDensity(float rest_density);

};

#endif //LIB_SPH_H

