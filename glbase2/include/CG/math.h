#pragma once

#include "glm/glm.hpp"

#define PI          3.141592653589793
#define INV_PI      0.3183098861837907
#define PI_2        1.5707963267948966
#define TAU         6.283185307179586
#define PI_180      0.017453292519943295
#define INV_PI_180  57.29577951308232
#define SQRT2       1.414213562373095
#define INV_SQRT2   0.7071067811865475
#define SQRT_3      1.732050807568877


namespace CG
{
	// Clamps x to [0, max) with repeating bounds
	inline float angle_clamp(float x, float max_x = TAU)
	{
		x = std::fmod(x, max_x); // x -> (-360, 360)
		if (x < 0)
			x += max_x; // -> [0, 360)
		return x;
	}
	
    // Converts a 3D vector to polar coordinates (r, theta, phi)
	// r: radius, theta: angle from +Y, phi: angle from +X in XZ plane
	inline glm::vec3 to_polar(glm::vec3 v) {
		float r = glm::length(v);

		if (r == 0.0)
			return glm::vec3(0.0);

		float theta = glm::acos(glm::clamp(v.y / r, -1.f, 1.f));
		float phi = glm::atan(v.z, v.x);

		return glm::vec3(r, theta, phi);
	}

    // Converts polar coordinates (r, theta, phi) to a 3D vector
	// r: radius, theta: angle from +Y, phi: angle from +X in XZ plane
	inline glm::vec3 from_polar(glm::vec3 p) {
		float r = p.x;
		float theta = p.y;
		float phi = p.z;

		float sin_th = glm::sin(theta);

		return r * glm::vec3(
			sin_th * glm::cos(phi),
			glm::cos(theta),
			sin_th * glm::sin(phi)
		);
	}
}

namespace glm
{
	// RH_NO
	template<typename T>
	GLM_FUNC_QUALIFIER glm::mat<4, 4, T, glm::defaultp> infinitePerspectiveReverseZ(T fovy, T aspect, T zNear)
	{
		T const range = glm::tan(fovy / static_cast<T>(2)) * zNear;
		T const left = -range * aspect;
		T const right = range * aspect;
		T const bottom = -range;
		T const top = range;

		glm::mat<4, 4, T, glm::defaultp> Result(static_cast<T>(0));
		Result[0][0] = (static_cast<T>(2) * zNear) / (right - left);
		Result[1][1] = (static_cast<T>(2) * zNear) / (top - bottom);
		Result[2][2] = static_cast<T>(1);
		Result[2][3] = -static_cast<T>(1);
		Result[3][2] = static_cast<T>(2) * zNear;
		return Result;
	}

	template<glm::length_t L, typename T>
	inline int max_axis(glm::vec<L, T> v)
	{
		int max_idx = 0;
		T max_val = glm::abs(v[0]);
		for (int i = 1; i < L; i++) {
			if (glm::abs(v[i]) > max_val) {
				max_val = glm::abs(v[i]);
				max_idx = i;
			}
		}
		return max_idx;
	}

	template<glm::length_t L, typename T>
	inline glm::vec<L, T> fmod(glm::vec<L, T> x, glm::vec<L, T> y)
	{
		glm::vec<L, T> out;
		for (int i = 0; i < L; i++) {
			out[i] = std::fmod(x[i], y[i]);
		}
		return out;
	}

	template<glm::length_t L, typename T>
	inline glm::vec<L, T> angle_clamp(glm::vec<L, T> x, float max_x = TAU)
	{
		glm::vec<L, T> out;
		for (int i = 0; i < L; i++) {
			out[i] = CG::angle_clamp(x[i], max_x);
		}
		return out;
	}
}