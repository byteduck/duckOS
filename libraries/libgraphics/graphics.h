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

#ifndef DUCKOS_LIBGRAPHICS_GRAPHICS_H
#define DUCKOS_LIBGRAPHICS_GRAPHICS_H

#include <sys/types.h>

__DECL_BEGIN

#define COLOR_A(color) (((color) & 0xFF000000) >> 24)
#define COLOR_R(color) (((color) & 0xFF0000) >> 16)
#define COLOR_G(color) (((color) & 0x00FF00) >> 8)
#define COLOR_B(color) ((color) & 0x0000FF)
#define RGB(r,g,b) (0xFF000000 | ((r) << 16) | ((g) << 8) | (b))
#define RGBA(r,g,b,a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

#define IMGSIZE(width, height) (sizeof(uint32_t) * (width) * (height))
#define IMGPIXEL(img, x, y) (img).data[(x) + (y) * (img).width]
#define IMGPTRPIXEL(img, x, y) (img)->buffer[(x) + (y) * (img)->width]

#include <sys/types.h>
#include "geometry.h"

class Font;
class Image {
public:
	Image();
	Image(uint32_t* buffer, int width, int height);

	uint32_t* data = nullptr;
	int width = 0;
	int height = 0;

	/**
	 * Copies a part of another image to this one.
	 * @param other The other image to copy from.
	 * @param other_area The area of the other image to copy.
	 * @param pos The position of this image to copy to.
	 */
	void copy(const Image& other, Rect other_area, const Point& pos) const;

	/**
	 * Copies part of another image with to one with alpha blending.
	 * @param other The other image to copy from.
	 * @param other_area The area of the other image to copy.
	 * @param pos The position of this image to copy to.
	 */
	void blend(const Image& other, Rect other_area, const Point& pos) const;

	/**
	 * Fills an area of the image with a color.
	 */
	void fill(Rect area, uint32_t color) const;

	/**
	 * Draws text on the image with a certain color.
	 */
	void draw_text(const char* str, const Point& point, Font* font, uint32_t color);

	/**
	 * Returns a pointer to the image buffer at a certain position. Returns NULL if outside the constraints.
	 */
	uint32_t* at(const Point& position) const;
};

__DECL_END

#endif //DUCKOS_LIBGRAPHICS_GRAPHICS_H
