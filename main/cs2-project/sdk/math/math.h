#pragma once
#include "vector.h"
#include <cmath>

#define M_DEG2RAD(DEGREES) ((DEGREES) * (math::_PI / 180.f))
#define M_RAD2DEG(RADIANS) ((RADIANS) * (180.f / math::_PI))
#define INTERVAL_PER_TICK 0.015625f
#define TICK_INTERVAL (INTERVAL_PER_TICK)
#define TIME_TO_TICKS(TIME) (static_cast<int>(0.5f + static_cast<float>(TIME) / TICK_INTERVAL))
#define TICKS_TO_TIME( t )	( TICK_INTERVAL *( t ) )
class matrix3x4_t;
class CBoneData;

namespace math
{
	inline constexpr float _PI = 3.141592654f;
	const float _pi2 = 2.0f * 3.14159265358979323846f;

	qangle_t CalcAngel(vec3_t viewPos, vec3_t aimPos);
	void vector_transform(vec3_t& in, matrix3x4_t& matrix, vec3_t& out);
	matrix3x4_t transform_to_matrix(CBoneData& in);
	vec3_t latency_adjust(const vec3_t& pos, const vec3_t& vel, float latency);
	vec3_t ExtrapolatePosition(const vec3_t& pos, const vec3_t& vel, float ticks);
	float rad2deg(float rad);
	float deg2rad(float deg);
	float normalize_yaw(float yaw) noexcept;
	vec3_t CalculateCameraPosition(vec3_t anchorPos, float distance, qangle_t viewAngles);
	qangle_t NormalizeAngles(qangle_t angles);
	void sin_cos(float rad, float* sine, float* cosine);
	vec3_t aim_direction(vec3_t src, vec3_t dst);
	void VectorAngles(const vec3_t& vecForward, qangle_t& vecAngles);
	void vector_angles4(vec3_t vec_forward, vec3_t& vec_angles);
	void angle_vectors(const vec3_t& angles, vec3_t& forward);
	void angle_vectors2(vec3_t angles, vec3_t& forward, vec3_t& right, vec3_t& up);
	void angle_vectors3(const vec3_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up);
	//void vector_angles(vec3_t vec_forward, vec3_t& vec_angles);
	template <typename T>
	constexpr const T& clamp(const T& value, const T& minimal, const T& maximal) noexcept
	{
		return (value < minimal) ? minimal : (value > maximal) ? maximal :
			value;
	}

	float rad(float gradus);
	inline int(__cdecl* fnRandomSeed)(int iSeed) = nullptr;
	inline float(__cdecl* fnRandomFloat)(float flMinValue, float flMaxValue) = nullptr;
	inline float(__cdecl* fnRandomFloatExp)(float flMinValue, float flMaxValue, float flExponent) = nullptr;
	inline int(__cdecl* fnRandomInt)(int iMinValue, int iMaxValue) = nullptr;
	inline float(__cdecl* fnRandomGaussianFloat)(float flMean, float flStdDev) = nullptr;
}
