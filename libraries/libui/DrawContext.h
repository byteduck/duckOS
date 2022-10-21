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

#pragma once

#include "Theme.h"
#include <libgraphics/Image.h>
#include <libgraphics/Graphics.h>

namespace UI {
	enum TextAlignment {
		BEGINNING, CENTER, END
	};

	class DrawContext {
	public:
		DrawContext(const Gfx::Framebuffer& framebuffer);

		///Properties
		int width() const;
		int height() const;
		Gfx::Dimensions dimensions() const;
		Gfx::Rect rect() const;
		const Gfx::Framebuffer& framebuffer() const;

		///Drawing
		void fill(Gfx::Rect rect, Color color) const;
		void fill_gradient_h(Gfx::Rect rect, Color color_a, Color color_b) const;
		void fill_gradient_v(Gfx::Rect rect, Color color_a, Color color_b) const;

		void draw_text(const char* str, Gfx::Rect rect, TextAlignment h_align, TextAlignment v_align, Gfx::Font* font, Color color) const;
		void draw_text(const char* str, Gfx::Point pos, Gfx::Font* font, Color color) const;
		void draw_text(const char* str, Gfx::Point pos, Color color) const;
		void draw_glyph(Gfx::Font* font, uint32_t codepoint, Gfx::Point pos, Color color) const;
		void draw_image(Duck::Ptr<const Gfx::Image> img, Gfx::Point pos) const;
		void draw_image(Duck::Ptr<const Gfx::Image> img, Gfx::Rect rect) const;
		void draw_image(const std::string& name, Gfx::Point pos) const;

		void draw_inset_rect(Gfx::Rect rect, Color bg, Color shadow_1, Color shadow_2, Color highlight) const;
		void draw_inset_rect(Gfx::Rect rect, Color bg) const;
		void draw_inset_rect(Gfx::Rect rect) const;

		void draw_outset_rect(Gfx::Rect rect, Color bg, Color shadow_1, Color shadow_2, Color highlight) const;
		void draw_outset_rect(Gfx::Rect rect, Color bg) const;
		void draw_outset_rect(Gfx::Rect rect) const;

		void draw_button_base(Gfx::Rect button, bool pressed) const;
		void draw_button(Gfx::Rect button, const std::string& text, bool pressed) const;
		void draw_button(Gfx::Rect button, const Gfx::Framebuffer& img, bool pressed) const;
		void draw_button(Gfx::Rect rect, Duck::Ptr<const Gfx::Image> img, bool pressed) const;

		void draw_vertical_scrollbar(Gfx::Rect area, Gfx::Rect handle_area, bool enabled) const;

		void draw_progressbar(Gfx::Rect area, double progress) const;

	private:
		const Gfx::Framebuffer* fb;
	};
}

