/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <cstdint>

typedef union Color {
public:
	struct {
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	} __attribute((packed));
	uint32_t value;

	inline Color(): b(0), g(0), r(0), a(0) {}
	inline Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a): b(b), g(g), r(r), a(a) {}
	inline Color(uint8_t r, uint8_t g, uint8_t b): b(b), g(g), r(r), a(255) {}
	inline Color(uint32_t raw_value): value(raw_value) {}
	inline operator uint32_t() { return value; }

} Color;

#define COLOR_A(color) (color.a)
#define COLOR_R(color) (color.r)
#define COLOR_G(color) (color.g)
#define COLOR_B(color) (color.b)
#define RGB(r,g,b) (Color {(uint8_t) (r), (uint8_t) (g), (uint8_t) (b), 255})
#define RGBA(r,g,b,a) (Color { (uint8_t) (r), (uint8_t) (g), (uint8_t) (b), (uint8_t) (a) })

#define COLOR_COMPONENT_MULT(a, b) ((uint8_t) (((unsigned) (a) *  (unsigned) (b) + 255 ) >> 8))
#define COLOR_MULT(a, b) \
	RGBA( \
			COLOR_COMPONENT_MULT(COLOR_R(a), COLOR_R(b)), \
			COLOR_COMPONENT_MULT(COLOR_G(a), COLOR_G(b)), \
			COLOR_COMPONENT_MULT(COLOR_B(a), COLOR_B(b)), \
			COLOR_COMPONENT_MULT(COLOR_A(a), COLOR_A(b)) \
		)
