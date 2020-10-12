/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#ifndef DUCKOS_POND_GRAPHICS_H
#define DUCKOS_POND_GRAPHICS_H

#include <sys/types.h>
#include "Geometry.h"

typedef struct Color {
public:
	Color(uint8_t r, uint8_t g, uint8_t b);
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
} Color;

class Framebuffer {
public:
	Color* buffer = nullptr;
	int width = 0;
	int height = 0;

	/**
	 * Copies a part of another framebuffer to this one.
	 * @param other The other framebuffer to copy from.
	 * @param other_area The area of the other framebuffer to copy.
	 * @param pos The position of this framebuffer to copy to.
	 */
	void copy(const Framebuffer& other, Rect other_area, const Point& pos) const;

	/**
	 * Fills an area of the framebuffer with a color.
	 */
	void fill(Rect area, const Color& color) const;

	/**
	 * Returns a pointer to the framebuffer at a certain position. Returns NULL if outside the constraints.
	 */
	Color* at(const Point& position) const;
};

#endif //DUCKOS_POND_GRAPHICS_H
