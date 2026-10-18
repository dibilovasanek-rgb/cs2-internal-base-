#pragma once
#include <cmath>
#include <numbers>
#include <algorithm>
#include <xmmintrin.h>
#include <pmmintrin.h> 
#include <iostream>
#include "..\..\dependencies\imgui\imgui.h"
#define M_RAD2DEGV(RADIANS) ((RADIANS) * (180.f / 3.141592654f))

struct vec2_t {
    float x{ }, y{ };

    vec2_t() { x = y = 0.0f; }

    vec2_t(const float fx, const float fy) {
        x = fx;
        y = fy;
    }

    vec2_t(float* v) {
        x = v[0];
        y = v[1];
    }

    vec2_t(const float* v) {
        x = v[0];
        y = v[1];
    }

    vec2_t(const vec2_t& v) {
        x = v.x;
        y = v.y;
    }

    ImVec2 im_vec2() const {
        return ImVec2(x, y);
    }

    vec2_t& operator=(const vec2_t& v) = default;
    float& operator[](const int i) { return reinterpret_cast<float*>(this)[i]; }
    float operator[](const int i) const { return ((float*)this)[i]; }

    vec2_t& operator+=(const vec2_t& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    vec2_t& operator-=(const vec2_t& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    vec2_t& operator*=(const vec2_t& v) {
        x *= v.x;
        y *= v.y;
        return *this;
    }

    vec2_t& operator/=(const vec2_t& v) {
        x /= v.x;
        y /= v.y;
        return *this;
    }

    vec2_t& operator+=(const float v) {
        x += v;
        y += v;
        return *this;
    }

    vec2_t& operator-=(const float v) {
        x -= v;
        y -= v;
        return *this;
    }

    vec2_t& operator*=(const float v) {
        x *= v;
        y *= v;
        return *this;
    }

    vec2_t& operator/=(const float v) {
        x /= v;
        y /= v;
        return *this;
    }

    vec2_t operator+(const vec2_t& v) const { return { x + v.x, y + v.y }; }
    vec2_t operator-(const vec2_t& v) const { return { x - v.x, y - v.y }; }
    vec2_t operator*(const vec2_t& v) const { return { x * v.x, y * v.y }; }
    vec2_t operator/(const vec2_t& v) const { return { x / v.x, y / v.y }; }
    vec2_t operator+(const float v) const { return { x + v, y + v }; }
    vec2_t operator-(const float v) const { return { x - v, y - v }; }
    vec2_t operator*(const float v) const { return { x * v, y * v }; }
    vec2_t operator/(const float v) const { return { x / v, y / v }; }
    operator bool() const noexcept { return !is_zero(); }

    [[nodiscard]] float length() const { return std::sqrt(x * x + y * y); }
    [[nodiscard]] float length_sqr() const { return (x * x + y * y); }
    [[nodiscard]] float dist_to(const vec2_t& v) const { return (*this - v).length(); }
    [[nodiscard]] float dist_to_sqr(const vec2_t& v) const { return (*this - v).length_sqr(); }
    [[nodiscard]] float dot(const vec2_t& v) const { return x * v.x + y * v.y; }

    [[nodiscard]] bool is_zero() const {
        return x > -FLT_EPSILON && x < FLT_EPSILON && y > -FLT_EPSILON && y < FLT_EPSILON;
    }
};

struct vec3_t;

struct qangle_t
{
    constexpr qangle_t(float x = 0.f, float y = 0.f, float z = 0.f) :
        x(x), y(y), z(z) {
    }

    constexpr qangle_t(const float* arrAngles) :
        x(arrAngles[0]), y(arrAngles[1]), z(arrAngles[2]) {
    }

#pragma region qangle_array_operators

    [[nodiscard]] float& operator[](const int nIndex)
    {
        return reinterpret_cast<float*>(this)[nIndex];
    }

    [[nodiscard]] const float& operator[](const int nIndex) const
    {
        return reinterpret_cast<const float*>(this)[nIndex];
    }

#pragma endregion

#pragma region qangle_relational_operators

    bool operator==(const qangle_t& angBase) const
    {
        return this->IsEqual(angBase);
    }

    bool operator!=(const qangle_t& angBase) const
    {
        return !this->IsEqual(angBase);
    }

#pragma endregion

#pragma region qangle_assignment_operators

    constexpr qangle_t& operator=(const qangle_t& angBase)
    {
        this->x = angBase.x;
        this->y = angBase.y;
        this->z = angBase.z;
        return *this;
    }

#pragma endregion

#pragma region qangle_arithmetic_assignment_operators

    constexpr qangle_t& operator+=(const qangle_t& angBase)
    {
        this->x += angBase.x;
        this->y += angBase.y;
        this->z += angBase.z;
        return *this;
    }

    constexpr qangle_t& operator-=(const qangle_t& angBase)
    {
        this->x -= angBase.x;
        this->y -= angBase.y;
        this->z -= angBase.z;
        return *this;
    }

    constexpr qangle_t& operator*=(const qangle_t& angBase)
    {
        this->x *= angBase.x;
        this->y *= angBase.y;
        this->z *= angBase.z;
        return *this;
    }

    constexpr qangle_t& operator/=(const qangle_t& angBase)
    {
        this->x /= angBase.x;
        this->y /= angBase.y;
        this->z /= angBase.z;
        return *this;
    }

    constexpr qangle_t& operator+=(const float flAdd)
    {
        this->x += flAdd;
        this->y += flAdd;
        this->z += flAdd;
        return *this;
    }

    constexpr qangle_t& operator-=(const float flSubtract)
    {
        this->x -= flSubtract;
        this->y -= flSubtract;
        this->z -= flSubtract;
        return *this;
    }

    constexpr qangle_t& operator*=(const float flMultiply)
    {
        this->x *= flMultiply;
        this->y *= flMultiply;
        this->z *= flMultiply;
        return *this;
    }

    constexpr qangle_t& operator/=(const float flDivide)
    {
        this->x /= flDivide;
        this->y /= flDivide;
        this->z /= flDivide;
        return *this;
    }

#pragma endregion

#pragma region qangle_arithmetic_unary_operators

    constexpr qangle_t& operator-()
    {
        this->x = -this->x;
        this->y = -this->y;
        this->z = -this->z;
        return *this;
    }

    constexpr qangle_t operator-() const
    {
        return { -this->x, -this->y, -this->z };
    }

#pragma endregion

#pragma region qangle_arithmetic_ternary_operators

    constexpr qangle_t operator+(const qangle_t& angAdd) const
    {
        return { this->x + angAdd.x, this->y + angAdd.y, this->z + angAdd.z };
    }

    constexpr qangle_t operator-(const qangle_t& angSubtract) const
    {
        return { this->x - angSubtract.x, this->y - angSubtract.y, this->z - angSubtract.z };
    }

    constexpr qangle_t operator*(const qangle_t& angMultiply) const
    {
        return { this->x * angMultiply.x, this->y * angMultiply.y, this->z * angMultiply.z };
    }

    constexpr qangle_t operator/(const qangle_t& angDivide) const
    {
        return { this->x / angDivide.x, this->y / angDivide.y, this->z / angDivide.z };
    }

    constexpr qangle_t operator+(const float flAdd) const
    {
        return { this->x + flAdd, this->y + flAdd, this->z + flAdd };
    }

    constexpr qangle_t operator-(const float flSubtract) const
    {
        return { this->x - flSubtract, this->y - flSubtract, this->z - flSubtract };
    }

    constexpr qangle_t operator*(const float flMultiply) const
    {
        return { this->x * flMultiply, this->y * flMultiply, this->z * flMultiply };
    }

    constexpr qangle_t operator/(const float flDivide) const
    {
        return { this->x / flDivide, this->y / flDivide, this->z / flDivide };
    }

#pragma endregion

    // @returns : true if each component of angle is finite, false otherwise
    [[nodiscard]] bool IsValid() const
    {
        return (std::isfinite(this->x) && std::isfinite(this->y) && std::isfinite(this->z));
    }

    /// @returns: true if each component of angle equals to another, false otherwise
    [[nodiscard]] bool IsEqual(const qangle_t& angEqual, const float flErrorMargin = std::numeric_limits<float>::epsilon()) const
    {
        return (std::fabsf(this->x - angEqual.x) < flErrorMargin && std::fabsf(this->y - angEqual.y) < flErrorMargin && std::fabsf(this->z - angEqual.z) < flErrorMargin);
    }

    /// @returns: true if each component of angle equals zero, false otherwise
    [[nodiscard]] bool IsZero() const
    {
        // @test: to make this implementation right, we should use fpclassify here, but game aren't doing same, probably it's better to keep this same, just ensure that it will be compiled same
        return (this->x == 0.0f && this->y == 0.0f && this->z == 0.0f);
    }

    /// @returns: length of hypotenuse
    [[nodiscard]] float Length2D() const
    {
        return std::sqrtf(x * x + y * y);
    }

    /// map polar angles to the range of [-180, 180] degrees
    /// @returns: normalized angle
    qangle_t& Normalize()
    {
        this->x = std::remainderf(this->x, 360.f);
        this->y = std::remainderf(this->y, 360.f);
        this->z = std::remainderf(this->z, 360.f);
        return *this;
    }

    constexpr qangle_t& Clamp()
    {
        this->x = std::clamp(this->x, -89.f, 89.f);
        this->y = std::clamp(std::remainder(this->y, 360.0f), -180.f, 180.f);
        this->z = 0.f;
        return *this;
    }

public:
    float x = 0.0f, y = 0.0f, z = 0.0f;
    void ToDirections(vec3_t* pvecForward, vec3_t* pvecRight = nullptr, vec3_t* pvecUp = nullptr) const;
};
constexpr float epsilon = 1e-6f;

class matrix3x4_t;
struct vec3_t {
    float x, y, z;

    vec3_t() : x(0.0f), y(0.0f), z(0.0f) {}
    vec3_t(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    vec3_t(float x_, float y_) : x(x_), y(y_), z(0.0f) {}
    constexpr vec3_t(const float* arrVector) :
        x(arrVector[0]), y(arrVector[1]), z(arrVector[2]) {
    }

    [[nodiscard]] float& operator[](const int nIndex)
    {
        return reinterpret_cast<float*>(this)[nIndex];
    }
    [[nodiscard]] const float& operator[](const int nIndex) const
    {
        return reinterpret_cast<const float*>(this)[nIndex];
    }
    __forceinline vec3_t operator+(const vec3_t& other) const {
        __m128 a = _mm_set_ps(0, z, y, x);
        __m128 b = _mm_set_ps(0, other.z, other.y, other.x);
        __m128 result = _mm_add_ps(a, b);
        return vec3_t(_mm_cvtss_f32(result),
            _mm_cvtss_f32(_mm_shuffle_ps(result, result, _MM_SHUFFLE(1, 1, 1, 1))),
            _mm_cvtss_f32(_mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 2, 2, 2))));
    }

    __forceinline vec3_t operator-(const vec3_t& other) const {
        if (reinterpret_cast<uintptr_t>(this) < 0x1000 || reinterpret_cast<uintptr_t>(&other) < 0x1000) {
            throw std::runtime_error("Invalid vec3_t object access.");
            return {};
        }

        __m128 a = _mm_set_ps(0, z, y, x);
        __m128 b = _mm_set_ps(0, other.z, other.y, other.x);
        __m128 result = _mm_sub_ps(a, b);

        return vec3_t(
            _mm_cvtss_f32(result),
            _mm_cvtss_f32(_mm_shuffle_ps(result, result, _MM_SHUFFLE(1, 1, 1, 1))),
            _mm_cvtss_f32(_mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 2, 2, 2)))
        );
    }


    __forceinline vec3_t operator*(float scalar) const {
        __m128 v = _mm_set_ps(0, z, y, x);
        __m128 s = _mm_set_ps1(scalar);
        __m128 result = _mm_mul_ps(v, s);
        return vec3_t(_mm_cvtss_f32(result),
            _mm_cvtss_f32(_mm_shuffle_ps(result, result, _MM_SHUFFLE(1, 1, 1, 1))),
            _mm_cvtss_f32(_mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 2, 2, 2))));
    }

    vec3_t& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    __forceinline vec3_t operator/(float scalar) const { return vec3_t(x / scalar, y / scalar, z / scalar); }
    __forceinline vec3_t& operator=(const vec3_t& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }
    __forceinline vec3_t operator-() const { return vec3_t(-x, -y, -z); }

    bool is_zero() const
    {
        return (this->x == 0.0f && this->y == 0.0f && this->z == 0.0f);
    }

    __forceinline float length() const {
        __m128 v = _mm_set_ps(0, z, y, x);
        __m128 sq = _mm_mul_ps(v, v);
        __m128 sum = _mm_hadd_ps(sq, sq);
        sum = _mm_hadd_ps(sum, sum);
        return _mm_cvtss_f32(_mm_sqrt_ss(sum));
    }

    __forceinline float length2d() const {
        __m128 v = _mm_set_ps(0, 0, y, x);
        __m128 sq = _mm_mul_ps(v, v);
        __m128 sum = _mm_hadd_ps(sq, sq);
        return _mm_cvtss_f32(_mm_sqrt_ss(sum));
    }

    __forceinline float length_squared() const { return x * x + y * y + z * z; }
    __forceinline void normalize() { float len = length(); if (len > epsilon) { x /= len; y /= len; z /= len; } }
    __forceinline vec3_t normalized() const { vec3_t result = *this; result.normalize(); return result; }
    __forceinline float dot(const vec3_t& other) const { return x * other.x + y * other.y + z * other.z; }
    float dot(vec3_t v, bool additional)
    {
        if (additional)
            return this->x * v.y + this->y * v.x + this->z * v.z;

        return this->x * v.x + this->y * v.y + this->z * v.z;
    }

    __forceinline vec3_t cross(const vec3_t& other) const {
        __m128 a = _mm_set_ps(0, z, y, x);
        __m128 b = _mm_set_ps(0, other.z, other.y, other.x);
        __m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
        __m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
        __m128 c = _mm_sub_ps(
            _mm_mul_ps(a, b_yzx),
            _mm_mul_ps(a_yzx, b)
        );
        return vec3_t(_mm_cvtss_f32(_mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1))),
            _mm_cvtss_f32(_mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 1, 0, 2))),
            _mm_cvtss_f32(_mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 2, 1, 0))));
    }

    __forceinline float normalize_inplace() {
        __m128 v = _mm_set_ps(0, z, y, x);
        __m128 sq = _mm_mul_ps(v, v);
        __m128 sum = _mm_hadd_ps(sq, sq);
        sum = _mm_hadd_ps(sum, sum);
        __m128 length = _mm_sqrt_ps(sum);
        __m128 inv_length = _mm_div_ps(_mm_set_ps1(1.0f), _mm_add_ps(length, _mm_set_ps1(std::numeric_limits<float>::epsilon())));
        __m128 normalized = _mm_mul_ps(v, inv_length);

        x = _mm_cvtss_f32(normalized);
        y = _mm_cvtss_f32(_mm_shuffle_ps(normalized, normalized, _MM_SHUFFLE(1, 1, 1, 1)));
        z = _mm_cvtss_f32(_mm_shuffle_ps(normalized, normalized, _MM_SHUFFLE(2, 2, 2, 2)));

        return _mm_cvtss_f32(length);
    }

    __forceinline float dist_to(const vec3_t& vec_end) const {
        return (*this - vec_end).length();
    }

    __forceinline float dot_absolute(const vec3_t& other) const {
        return std::fabs(x * other.x) + std::fabs(y * other.y) + std::fabs(z * other.z);
    }

    void clamp(float minVal, float maxVal)
    {
       x = (x < minVal) ? minVal : (x > maxVal) ? maxVal : x;
       y = (y < minVal) ? minVal : (y > maxVal) ? maxVal : y;
       z = (z < minVal) ? minVal : (z > maxVal) ? maxVal : z;
    }

    vec3_t& clamp() {
        x = std::clamp(x, -89.f, 89.f);
        y = std::clamp(std::remainder(y, 360.0f), -180.f, 180.f);
        z = 0.f;

        return *this;
    }
    float normalize_in_place();

    qangle_t ToAngles() const
    {
        float flPitch, flYaw;
        if (this->x == 0.0f && this->y == 0.0f)
        {
            flPitch = (this->z > 0.0f) ? 270.f : 90.f;
            flYaw = 0.0f;
        }
        else
        {
            flPitch = M_RAD2DEGV(std::atan2f(-this->z, this->length2d()));

            if (flPitch < 0.f)
                flPitch += 360.f;

            flYaw = M_RAD2DEGV(std::atan2f(this->y, this->x));

            if (flYaw < 0.f)
                flYaw += 360.f;
        }

        return { flPitch, flYaw, 0.0f };
    }
    vec3_t transform(matrix3x4_t trans);
};

struct vec4_t
{
    constexpr vec4_t(const float x = 0.0f, const float y = 0.0f, const float z = 0.0f, const float w = 0.0f) :
        x(x), y(y), z(z), w(w) { }

    vec4_t(const float* clr)
    {
        x = clr[0];
        y = clr[1];
        z = clr[2];
        w = clr[3];
    }

    void init(const float ix = 0.0f, const float iy = 0.0f, const float iz = 0.0f, const float iw = 0.0f)
    {
        x = ix;
        y = iy;
        z = iz;
        w = iw;
    }

    float x{ }, y{ }, z{ }, w{ };
};

struct __declspec(align(16)) vector_aligned_t : vec3_t {
    vector_aligned_t() = default;
    vector_aligned_t(const float x, const float y, const float z) { vec3_t(x, y, z); }
    explicit vector_aligned_t(const vec3_t& other) { vec3_t(other.x, other.y, other.z); }

    vector_aligned_t& operator=(const vec3_t& other) {
        vec3_t(other.x, other.y, other.z);
        return *this;
    }

    vector_aligned_t& operator=(const vector_aligned_t& other) {
        vec3_t(other.x, other.y, other.z);
        return *this;
    }

    float w{ };
};
