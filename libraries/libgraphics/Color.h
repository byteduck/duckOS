/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <cstdint>
#include <algorithm>

namespace Gfx {
typedef union Color {
public:
	struct {
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	} __attribute((packed));
	uint32_t value;

	constexpr Color(): b(0), g(0), r(0), a(0) {}
	constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a): b(b), g(g), r(r), a(a) {}
	constexpr Color(uint8_t r, uint8_t g, uint8_t b): b(b), g(g), r(r), a(255) {}
	constexpr Color(uint32_t raw_value): value(raw_value) {}
	inline operator uint32_t() { return value; }

	[[nodiscard]] inline constexpr Color blended(Color other) const {
		if (!other.value || !other.a)
			return *this;
		else if(other.a == 255)
			return other;
		unsigned int alpha = other.a + 1;
		unsigned int inv_alpha = 256 - other.a;
		return {
			(uint8_t) ((alpha * other.r + inv_alpha * r) >> 8),
			(uint8_t) ((alpha * other.g + inv_alpha * g) >> 8),
			(uint8_t) ((alpha * other.b + inv_alpha * b) >> 8),
			(uint8_t) ((alpha * other.a + inv_alpha * a) >> 8)
		};
	}

	[[nodiscard]] constexpr Color operator*(Color other) const {
		return {
				((uint8_t) (((int) r * (int) other.r + 255 ) >> 8)),
				((uint8_t) (((int) g * (int) other.g + 255 ) >> 8)),
				((uint8_t) (((int) b * (int) other.b + 255 ) >> 8)),
				((uint8_t) (((int) a * (int) other.a + 255 ) >> 8)),
		};
	}

	constexpr Color operator*=(Color other) {
		return *this = *this * other;
	}

	[[nodiscard]] constexpr Color lightened(float amount = 0.2) const {
		amount = 1 + amount;
		return Color((uint8_t) std::min(((float) r * amount), 255.0f),
					 (uint8_t) std::min(((float) g * amount), 255.0f),
					 (uint8_t) std::min(((float) b * amount), 255.0f),
					 a);
	}

	[[nodiscard]] constexpr Color darkened(float amount = 0.2) const {
		amount = 1 - amount;
		return Color((uint8_t) std::min(((float) r * amount), 255.0f),
					 (uint8_t) std::min(((float) g * amount), 255.0f),
					 (uint8_t) std::min(((float) b * amount), 255.0f),
					 a);
	}

	[[nodiscard]] constexpr Color mixed(Color other, float percent) const {
		float oneminus = 1.0 - percent;
		return Color(
			(uint8_t)(r * oneminus + other.r * percent),
			(uint8_t)(g * oneminus + other.g * percent),
			(uint8_t)(b * oneminus + other.b * percent),
			(uint8_t)(a * oneminus + other.a * percent)
		);
	}

	[[nodiscard]] constexpr Color inverted() const {
		return Color(255 - r, 255 - g, 255 - b, a);
	}

} Color;
}

#define COLOR_A(color) (color.a)
#define COLOR_R(color) (color.r)
#define COLOR_G(color) (color.g)
#define COLOR_B(color) (color.b)
#define RGB(r,g,b) (Gfx::Color {(uint8_t) (r), (uint8_t) (g), (uint8_t) (b), 255})
#define RGBA(r,g,b,a) (Gfx::Color { (uint8_t) (r), (uint8_t) (g), (uint8_t) (b), (uint8_t) (a) })
