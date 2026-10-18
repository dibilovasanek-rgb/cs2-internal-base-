#include "math.h"
#include <DirectXMath.h>
#include <algorithm>
#include "matrix.h"
#include "../game_class/c_model.h"

qangle_t math::CalcAngel(vec3_t viewPos, vec3_t aimPos)
{
	__m128 view = _mm_set_ps(0.0f, viewPos.z, viewPos.y, viewPos.x);
	__m128 aim = _mm_set_ps(0.0f, aimPos.z, aimPos.y, aimPos.x);
	__m128 delta = _mm_sub_ps(aim, view);

	__m128 squared = _mm_mul_ps(delta, delta);
	__m128 sum = _mm_hadd_ps(squared, squared);
	sum = _mm_hadd_ps(sum, sum);
	__m128 length = _mm_sqrt_ps(sum);

	float deltaZ = _mm_cvtss_f32(_mm_shuffle_ps(delta, delta, _MM_SHUFFLE(2, 2, 2, 2)));
	float deltaY = _mm_cvtss_f32(_mm_shuffle_ps(delta, delta, _MM_SHUFFLE(1, 1, 1, 1)));
	float deltaX = _mm_cvtss_f32(delta);

	qangle_t angle;
	angle.x = -asinf(deltaZ / _mm_cvtss_f32(length)) * 57.2957795131f;
	angle.y = atan2f(deltaY, deltaX) * 57.2957795131f;
	angle.z = 0.0f;

	return angle;
}

void math::vector_transform(vec3_t& in, matrix3x4_t& matrix, vec3_t& out) {
	out = {
		in.dot({ matrix[0][0], matrix[0][1], matrix[0][2] }) + matrix[0][3],
		in.dot({ matrix[1][0], matrix[1][1], matrix[1][2] }) + matrix[1][3],
		in.dot({ matrix[2][0], matrix[2][1], matrix[2][2] }) + matrix[2][3]
	};
}

matrix3x4_t math::transform_to_matrix(CBoneData& in) {
	matrix3x4_t  matrix;

	vec4_t rot;
	rot = in.rotation;
	vec3_t pos;
	pos = in.position;

	matrix[0][0] = 1.0f - 2.0f * rot.y * rot.y - 2.0f * rot.z * rot.z;
	matrix[1][0] = 2.0f * rot.x * rot.y + 2.0f * rot.w * rot.z;
	matrix[2][0] = 2.0f * rot.x * rot.z - 2.0f * rot.w * rot.y;

	matrix[0][1] = 2.0f * rot.x * rot.y - 2.0f * rot.w * rot.z;
	matrix[1][1] = 1.0f - 2.0f * rot.x * rot.x - 2.0f * rot.z * rot.z;
	matrix[2][1] = 2.0f * rot.y * rot.z + 2.0f * rot.w * rot.x;

	matrix[0][2] = 2.0f * rot.x * rot.z + 2.0f * rot.w * rot.y;
	matrix[1][2] = 2.0f * rot.y * rot.z - 2.0f * rot.w * rot.x;
	matrix[2][2] = 1.0f - 2.0f * rot.x * rot.x - 2.0f * rot.y * rot.y;

	matrix[0][3] = pos.x;
	matrix[1][3] = pos.y;
	matrix[2][3] = pos.z;

	return matrix;
}

vec3_t math::latency_adjust(const vec3_t& pos, const vec3_t& vel, float latency) {
	__m128 position = _mm_set_ps(0.0f, pos.z, pos.y, pos.x);
	__m128 velocity = _mm_set_ps(0.0f, vel.z, vel.y, vel.x);
	__m128 latencyScalar = _mm_set_ps1(latency);

	__m128 adjusted = _mm_add_ps(
		position,
		_mm_mul_ps(velocity, latencyScalar)
	);

	vec3_t result;
	_mm_store_ps((float*)&result, adjusted);
	return result;
}

vec3_t math::ExtrapolatePosition(const vec3_t& pos, const vec3_t& vel, float ticks) {
	__m128 position = _mm_set_ps(0.0f, pos.z, pos.y, pos.x);
	__m128 velocity = _mm_set_ps(0.0f, vel.z, vel.y, vel.x);
	__m128 tickScalar = _mm_set_ps1(0.015625f * ticks);

	__m128 extrapolated = _mm_add_ps(
		position,
		_mm_add_ps(
			_mm_mul_ps(velocity, tickScalar),
			velocity
		)
	);

	vec3_t result;
	_mm_store_ps((float*)&result, extrapolated);
	return result;
}

float math::rad2deg(float rad)
{
	float result = rad * (180.0f / _PI);
	return result;
}

float math::deg2rad(float deg)
{
	float result = deg * (_PI / 180.0f);
	return result;
}

[[nodiscard]] float math::normalize_yaw(float yaw) noexcept
{
	while (yaw > 180.0f)
		yaw -= 360.0f;

	while (yaw < -180.0f)
		yaw += 360.0f;

	return yaw;
}

vec3_t math::CalculateCameraPosition(vec3_t anchorPos, float distance, qangle_t viewAngles)
{
	float yaw = DirectX::XMConvertToRadians(viewAngles.y);
	float pitch = DirectX::XMConvertToRadians(viewAngles.x);

	float x = anchorPos.x + distance * cosf(yaw) * cosf(pitch);
	float y = anchorPos.y + distance * sinf(yaw) * cosf(pitch);
	float z = anchorPos.z + distance * sinf(pitch);

	return vec3_t{ x, y, z };
}

void math::sin_cos(float rad, float* sine, float* cosine)
{
	*sine = std::sinf(rad);
	*cosine = std::cosf(rad);
}

vec3_t math::aim_direction(vec3_t src, vec3_t dst)
{	
	vec3_t result;

	vec3_t delta = dst - src;

	vector_angles4(delta, result);

	return result;
}

void math::VectorAngles(const vec3_t& vecForward, qangle_t& vecAngles) {
	vec3_t vecView;
	if (vecForward.y == 0 && vecForward.x == 0)
	{
		vecView.x = 0.f;
		vecView.y = 0.f;
	}
	else
	{
		vecView.y = atan2(vecForward.y, vecForward.x) * 180.f / 3.14f;

		if (vecView.y < 0.f)
			vecView.y += 360.f;

		auto tmp = vecForward.length2d();

		vecView.x = atan2(-vecForward.z, tmp) * 180.f / 3.14f;

		if (vecView.x < 0.f)
			vecView.x += 360.f;
	}

	vecAngles.x = vecView.x;
	vecAngles.y = vecView.y;
	vecAngles.z = 0.f;
}

void math::vector_angles4(vec3_t vec_forward, vec3_t& vec_angles)
{
	if (vec_forward.x == 0.f && vec_forward.y == 0.f)
	{
		vec_angles.y = 0.f;
		vec_angles.x = vec_forward.z > 0.f ? 270.f : 90.f;
	}
	else
	{
		vec_angles.y = rad2deg(std::atan2(vec_forward.y, vec_forward.x));
		if (vec_angles.y < 0.f)
			vec_angles.y += 360.f;

		vec_angles.x = rad2deg(std::atan2(-vec_forward.z, vec_forward.length2d()));
		if (vec_angles.x < 0.f)
			vec_angles.x += 360.f;
	}

	vec_angles.x = std::remainder(vec_angles.x, 360.f);
	vec_angles.y = std::remainder(vec_angles.y, 360.f);
	vec_angles.z = std::remainder(vec_angles.z, 360.f);
}

void math::angle_vectors(const vec3_t& angles, vec3_t& forward)
{
	float sp, sy, cp, cy;

	sin_cos(deg2rad(angles[1]), &sy, &cy);
	sin_cos(deg2rad(angles[0]), &sp, &cp);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

float math::rad(float gradus)
{
	return gradus * (180.f / 3.1415);
}

void math::angle_vectors2(vec3_t angles, vec3_t& forward, vec3_t& right, vec3_t& up)
{
	float			angle;
	static float	sr, sp, sy, cr, cp, cy;

	angle = angles.y * (_pi2 / 360);
	sy = sin(angle);
	cy = cos(angle);

	angle = angles.x * (_pi2 / 360);
	sp = sin(angle);
	cp = cos(angle);

	angle = angles.z * (_pi2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;

	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr * cy);
	right.z = -1 * sr * cp;

	up.x = (cr * sp * cy + -sr * -sy);
	up.y = (cr * sp * sy + -sr * cy);
	up.z = cr * cp;
}

void math::angle_vectors3(const vec3_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up)
{
	float sp, sy, sr, cp, cy, cr;

	sin_cos(angles.x * 0.017453292f, &sp, &cp);
	sin_cos(angles.y * 0.017453292f, &sy, &cy);
	sin_cos(angles.z * 0.017453292f, &sr, &cr);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = sr * sp * cy - cr * sy;
		right->y = cr * cy + sr * sp * sy;
		right->z = sr * cp;
	}

	if (up)
	{
		up->x = sr * sy + cr * sp * cy;
		up->y = cr * sp * sy - sr * cy;
		up->z = cr * cp;
	}
}


void qangle_t::ToDirections(vec3_t* pvecForward, vec3_t* pvecRight, vec3_t* pvecUp) const
{
	float flPitchSin, flPitchCos, flYawSin, flYawCos, flRollSin, flRollCos;
	DirectX::XMScalarSinCos(&flPitchSin, &flPitchCos, M_DEG2RAD(this->x));
	DirectX::XMScalarSinCos(&flYawSin, &flYawCos, M_DEG2RAD(this->y));
	DirectX::XMScalarSinCos(&flRollSin, &flRollCos, M_DEG2RAD(this->z));

	if (pvecForward != nullptr)
	{
		pvecForward->x = flPitchCos * flYawCos;
		pvecForward->y = flPitchCos * flYawSin;
		pvecForward->z = -flPitchSin;
	}

	if (pvecRight != nullptr)
	{
		pvecRight->x = (-flRollSin * flPitchSin * flYawCos) + (-flRollCos * -flYawSin);
		pvecRight->y = (-flRollSin * flPitchSin * flYawSin) + (-flRollCos * flYawCos);
		pvecRight->z = (-flRollSin * flPitchCos);
	}

	if (pvecUp != nullptr)
	{
		pvecUp->x = (flRollCos * flPitchSin * flYawCos) + (-flRollSin * -flYawSin);
		pvecUp->y = (flRollCos * flPitchSin * flYawSin) + (-flRollSin * flYawCos);
		pvecUp->z = (flRollCos * flPitchCos);
	}
}

float vec3_t::normalize_in_place()
{
	const float length = this->length();
	const float radius = 1.0f / (length + FLT_EPSILON);

	x *= radius;
	y *= radius;
	z *= radius;

	return length;
}

qangle_t math::NormalizeAngles(qangle_t angles)
{
	while (angles.x > 89.0f)
		angles.x -= 180.0f;
	while (angles.x < -89.0f)
		angles.x += 180.0f;
	while (angles.y > 180.0f)
		angles.y -= 360.0f;
	while (angles.y < -180.0f)
		angles.y += 360.0f;
	angles.z = 0.0f;
	return angles;
}
