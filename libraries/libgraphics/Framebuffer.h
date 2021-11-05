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

    Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#ifndef DUCKOS_LIBGRAPHICS_FRAMEBUFFER_H
#define DUCKOS_LIBGRAPHICS_FRAMEBUFFER_H

#include <sys/types.h>
#include "geometry.h"

#define IMGSIZE(width, height) (sizeof(uint32_t) * (width) * (height))
#define IMGPIXEL(img, x, y) (img).data[(x) + (y) * (img).width]
#define IMGPTRPIXEL(img, x, y) (img)->data[(x) + (y) * (img)->width]

namespace Gfx {
	class Font;
	class Framebuffer {
	public:
		Framebuffer();
		Framebuffer(uint32_t* buffer, int width, int height);

		uint32_t* data = nullptr;
		int width = 0;
		int height = 0;

		/**
		 * Frees the data associated with the Image.
		 */
		void free();

		/**
		 * Copies a part of another Image to this one.
		 * @param other The other Image to copy from.
		 * @param other_area The area of the other Image to copy.
		 * @param pos The position of this Image to copy to.
		 */
		void copy(const Framebuffer& other, Rect other_area, const Point& pos) const;

		/**
		 * Copies a part of another Image to this one, ignoring alpha.
		 * @param other The other Image to copy from.
		 * @param other_area The area of the other Image to copy.
		 * @param pos The position of this Image to copy to.
		 */
		void copy_noalpha(const Framebuffer& other, Rect other_area, const Point& pos) const;

		/**
		 * Copies a part of another Image to this one, with alpha blending.
		 * @param other The other Image to copy from.
		 * @param other_area The area of the other Image to copy.
		 * @param pos The position of this Image to copy to.
		 */
		void copy_blitting(const Framebuffer& other, Rect other_area, const Point& pos) const;

		/**
		 * Identical to ::copy(), but will tile the Image.
		 */
		void copy_tiled(const Framebuffer& other, Rect other_area, const Point& pos) const;

		/**
		 * Draws another Image on top of this one with alpha blending.
		 * @param other The Image to draw.
		 * @param other_area The area of the other Image to draw.
		 * @param pos The position on this Image to draw on.
		 */
		void draw_image(const Framebuffer& other, Rect other_area, const Point& pos) const;

		/**
		 * Draws another Image on top of this one with alpha blending.
		 * @param other The Image to draw.
		 * @param pos The position on this Image to draw on.
		 */
		void draw_image(const Framebuffer& other, const Point& pos) const;

        /**
		 * Draws another Image on top of this one with alpha blending, scaled to
         * fit inside of the specified rect.
		 * @param other The Image to draw.
         * @param size The rect on this Image to scale the image to and draw on.
		 */
        void draw_image_scaled(const Framebuffer& other, const Rect& rect) const;

		/**
		 * Fills an area of the Image with a color.
		 * @param area The area to fill.
		 * @param color The color to fill the area with.
		 */
		void fill(Rect area, uint32_t color) const;

		/**
		 * Draws the outline of an area on the Image.
		 * @param area The rect of the area to outline.
		 * @param color The color to draw the outline in.
		 */
		void outline(Rect area, uint32_t color) const;

		/**
		 * Draws text on the Image with a certain color.
		 * @param str The string to draw.
		 * @param pos The top-left position of where to draw.
		 * @param font The font to use.
		 * @param color The color to draw in.
		 */
		void draw_text(const char* str, const Point& pos, Font* font, uint32_t color) const;

		/**
		 * Draws a glyph on the Image with a certain color.
		 * @param font The font to use.
		 * @param codepoint The codepoint of the character.
		 * @param pos The position to draw the glyph at.
		 * @param color The color to draw the glyph in.
		 * @return The position where the next character should be drawn.
		 */
		Point draw_glyph(Font* font, uint32_t codepoint, const Point& pos, uint32_t color) const;

		/**
		 * Multiplies the image with a certain color.
		 * @param color The color to multiply by.
		 */
		void multiply(uint32_t color);

		/**
		 * Returns a pointer to the Image buffer at a certain position. Returns NULL if outside the constraints.
		 */
		uint32_t* at(const Point& position) const;
	};
}


#endif //DUCKOS_LIBGRAPHICS_FRAMEBUFFER_H
