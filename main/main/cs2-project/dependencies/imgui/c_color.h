#pragma once

#include <array>
#include <cmath>
#include "imgui.h"

class c_byte_color {
public:
	unsigned char r, g, b;

	c_byte_color(float _r = 0, float _g = 0, float _b = 0) {
		r = static_cast<unsigned char>(_r);
		g = static_cast<unsigned char>(_g);
		b = static_cast<unsigned char>(_b);
	}

	c_byte_color operator+(c_byte_color color) {
		return c_byte_color(r + color.r, g + color.g, b + color.b);
	}

	c_byte_color operator+(unsigned char n) {
		return c_byte_color(r + n, g + n, b + n);
	}

	c_byte_color operator+=(c_byte_color color) {
		r += color.r;
		g += color.g;
		b += color.b;

		return *this;
	}

	c_byte_color operator+=(unsigned char n) {
		r += n;
		g += n;
		b += n;

		return *this;
	}

	c_byte_color operator-(c_byte_color color) {
		return c_byte_color(r - color.r, g - color.g, b - color.b);
	}

	c_byte_color operator-(unsigned char n) {
		return c_byte_color(r - n, g - n, b - n);
	}

	c_byte_color operator-=(c_byte_color color) {
		r -= color.r;
		g -= color.g;
		b -= color.b;

		return *this;
	}

	c_byte_color operator-=(unsigned char n) {
		r -= n;
		g -= n;
		b -= n;

		return *this;
	}

	c_byte_color operator/(c_byte_color color) {
		return c_byte_color(r / color.r, g / color.g, b / color.b);
	}

	c_byte_color operator/(unsigned char n) {
		return c_byte_color(r / n, g / n, b / n);
	}

	c_byte_color operator/=(c_byte_color color) {
		r /= color.r;
		g /= color.g;
		b /= color.b;

		return *this;
	}

	c_byte_color operator/=(unsigned char n) {
		r /= n;
		g /= n;
		b /= n;

		return *this;
	}

	c_byte_color operator*(c_byte_color color) {
		return c_byte_color(r * color.r, g * color.g, b * color.b);
	}

	c_byte_color operator*(unsigned char n) {
		return c_byte_color(r * n, g * n, b * n);
	}

	c_byte_color operator*=(c_byte_color color) {
		r *= color.r;
		g *= color.g;
		b *= color.b;

		return *this;
	}

	c_byte_color operator*=(unsigned char n) {
		r *= n;
		g *= n;
		b *= n;

		return *this;
	}

	bool operator==(c_byte_color color) {
		return r == color.r && g == color.g && b == color.b;
	}

	bool operator!=(c_byte_color color) {
		return !(*this == color);
	}
};

class c_color {
public:
	float r, g, b, a;

	c_color(float _r = 0, float _g = 0, float _b = 0, float _a = 0) {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}

	c_color operator+(c_color color) {
		return c_color(r + color.r, g + color.g, b + color.b, a + color.a);
	}

	c_color operator+(float n) {
		return c_color(r + n, g + n, b + n, a + n);
	}

	c_color alpha(float new_a) const {
		return c_color(r, g, b, new_a);
	}

	c_color operator+=(c_color color) {
		r += color.r;
		g += color.g;
		b += color.b;
		a += color.a;

		return *this;
	}


	void BaseAlpha(float out[4]) const
	{
		out[0] = r / 255.0f;
		out[1] = g / 255.0f;
		out[2] = b / 255.0f;
		out[3] = a / 255.0f;
	}


	static c_color FromBase4(const float in[4])
	{
		return { in[0], in[1], in[2], in[3] };
	}

	c_color operator+=(float n) {
		r += n;
		g += n;
		b += n;
		a += n;

		return *this;
	}

	[[nodiscard]] constexpr ImU32 GetU32(const float flAlphaMultiplier = 1.0f) const
	{
		return IM_COL32(r, g, b, a * flAlphaMultiplier);
	}

	c_color operator-(c_color color) {
		return c_color(r - color.r, g - color.g, b - color.b, a - color.a);
	}

	c_color operator-(float n) {
		return c_color(r - n, g - n, b - n, a - n);
	}

	c_color operator-=(c_color color) {
		r -= color.r;
		g -= color.g;
		b -= color.b;
		a -= color.a;

		return *this;
	}

	c_color operator-=(float n) {
		r -= n;
		g -= n;
		b -= n;
		a -= n;

		return *this;
	}

	c_color operator/(c_color color) {
		return c_color(r / color.r, g / color.g, b / color.b, a / color.a);
	}

	c_color operator/(float n) {
		return c_color(r / n, g / n, b / n, a / n);
	}

	c_color operator/=(c_color color) {
		r /= color.r;
		g /= color.g;
		b /= color.b;
		a /= color.a;

		return *this;
	}

	c_color operator/=(float n) {
		r /= n;
		g /= n;
		b /= n;
		a /= n;

		return *this;
	}

	c_color operator*(c_color color) {
		return c_color(r * color.r, g * color.g, b * color.b, a * color.a);
	}

	c_color operator*(float n) {
		return c_color(r * n, g * n, b * n, a * n);
	}

	c_color operator*=(c_color color) {
		r *= color.r;
		g *= color.g;
		b *= color.b;
		a *= color.a;

		return *this;
	}

	c_color operator*=(float n) {
		r *= n;
		g *= n;
		b *= n;
		a *= n;

		return *this;
	}

	bool operator==(c_color color) {
		return r == color.r && g == color.g && b == color.b && a == color.a;
	}

	bool operator!=(c_color color) {
		return !(*this == color);
	}

	c_color convert_from_hsv_to_color() {
		r /= 255.f;
		g /= 255.f;
		b /= 255.f;
		a /= 255.f;
		return *this;
	}

	c_color lerp(c_color& other, float step) {
		c_color lerped_color{
			std::lerp(r, other.r, step),
			std::lerp(g, other.g, step),
			std::lerp(b, other.b, step),
			std::lerp(a, other.a, step)
		};

		return lerped_color;
	}

	c_byte_color to_byte() {
		return c_byte_color(r, g, b);
	}

	ImColor im()
	{
		return ImColor(r, g, b, a);
	}
};