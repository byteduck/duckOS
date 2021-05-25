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

#ifndef DUCKOS_LIBUI_DRAWCONTEXT_H
#define DUCKOS_LIBUI_DRAWCONTEXT_H

#include "Theme.h"
#include <libgraphics/graphics.h>

namespace UI {
    enum TextAlignment {
        BEGINNING, CENTER, END
    };

	class DrawContext {
	public:
		DrawContext(const Image& framebuffer);

		///Properties
		int width() const;
		int height() const;
		const Image& framebuffer() const;

		///Drawing
		void fill(Rect rect, Color color) const;
		void fill_gradient_h(Rect rect, Color color_a, Color color_b) const;

		void draw_text(const char* str, Rect rect, TextAlignment h_align, TextAlignment v_align, Font* font, Color color) const;
		void draw_text(const char* str, Point pos, Font* font, Color color) const;
		void draw_text(const char* str, Point pos, Color color) const;
		void draw_glyph(Font* font, uint32_t codepoint, Point pos, Color color) const;
		void draw_image(const Image& img, Point pos) const;
		void draw_image(const std::string& name, Point pos) const;
		void draw_image(const Image& img, Rect img_area, Point pos) const;
		void draw_image(const std::string& name, Rect img_area, Point pos) const;

		void draw_inset_rect(Rect rect, Color bg, Color shadow_1, Color shadow_2, Color highlight) const;
		void draw_inset_rect(Rect rect, Color bg) const;
		void draw_inset_rect(Rect rect) const;

		void draw_outset_rect(Rect rect, Color bg, Color shadow_1, Color shadow_2, Color highlight) const;
		void draw_outset_rect(Rect rect, Color bg) const;
		void draw_outset_rect(Rect rect) const;

		void draw_button_base(Rect button, bool pressed) const;
		void draw_button(Rect button, const std::string& text, bool pressed) const;
		void draw_button(Rect button, const Image& img, bool pressed) const;

	private:
		const Image* fb;
	};
}

#endif //DUCKOS_LIBUI_DRAWCONTEXT_H
