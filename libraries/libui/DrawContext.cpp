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

#include "DrawContext.h"
#include <libgraphics/Image.h>
#include <libgraphics/Font.h>

using namespace Gfx;
using namespace Duck;

UI::DrawContext::DrawContext(const Framebuffer& framebuffer): fb(&framebuffer) {

}

int UI::DrawContext::width() const {
	return fb->width;
}

int UI::DrawContext::height() const {
	return fb->height;
}

Dimensions UI::DrawContext::dimensions() const {
	return { fb->width, fb->height };
}

Rect UI::DrawContext::rect() const {
	return { 0, 0, fb->width, fb->height };
}

const Framebuffer& UI::DrawContext::framebuffer() const {
	return *fb;
}

void UI::DrawContext::fill(Gfx::Rect rect, Color color) const {
	fb->fill(rect, color);
}

void UI::DrawContext::fill_gradient_h(Gfx::Rect rect, Color color_a, Color color_b) const {
	fb->fill_gradient_h(rect, color_a, color_b);
}

void UI::DrawContext::fill_gradient_v(Gfx::Rect rect, Color color_a, Color color_b) const {
	fb->fill_gradient_v(rect, color_a, color_b);
}

void UI::DrawContext::draw_text(const char* str, Gfx::Rect rect, TextAlignment h_align, TextAlignment v_align, Font* font, Color color) const {
	// First, split up the text into lines
	struct Line {
		std::string text;
		Dimensions bounds;
	};

	Point cur_pos = rect.position();
	std::vector<Line> lines;
	Line cur_line;
	Dimensions total_dimensions {0, 0};

	auto finalize_line = [&] {
		cur_line.bounds = { font->size_of(cur_line.text.c_str()).width, font->bounding_box().height };
		lines.push_back(cur_line);
		cur_line.text.clear();
		total_dimensions.width = std::max(cur_line.bounds.width, total_dimensions.width);
		total_dimensions.height += cur_line.bounds.height;
		cur_pos = {rect.x, cur_pos.y + font->bounding_box().height};
	};

	auto rect_for_glyph = [&](FontGlyph* glyph) {
		return Rect {
				glyph->base_x - font->bounding_box().base_x + cur_pos.x,
				(font->bounding_box().base_y - glyph->base_y) + (font->size() - glyph->height) + cur_pos.y,
				glyph->width,
				glyph->height
		};
	};

	while(*str) {
		auto glyph = font->glyph(*str);
		if(!rect_for_glyph(glyph).inside(rect)) {
			// We ran out of space on the line, try line wrapping
			finalize_line();
			if(!rect_for_glyph(glyph).inside(rect))
				break;
		}

		cur_line.text += *str;
		auto off = font->glyph(*str)->next_offset;
		cur_pos += {off.x, off.y};
		str++;
	}

	if(!cur_line.text.empty())
		finalize_line();

	// Next, apply vertical alignment
	Point text_pos = rect.position();
	switch(v_align) {
		case BEGINNING:
			break;
		case CENTER:
			text_pos.y = rect.y + rect.height / 2 - total_dimensions.height / 2;
			break;
		case END:
			text_pos.y = rect.y + rect.height - total_dimensions.height;
			break;
	}

	// Then, draw all the lines
	for(auto& line : lines) {
		switch(h_align) {
			case BEGINNING:
				break;
			case CENTER:
				text_pos.x = rect.x + rect.width / 2 - line.bounds.width / 2;
				break;
			case END:
				text_pos.x = rect.x + rect.width - line.bounds.width;
				break;
		}

		fb->draw_text(line.text.c_str(), text_pos, font, color);
		text_pos = {rect.position().x, text_pos.y + font->bounding_box().height};
	}
}

void UI::DrawContext::draw_text(const char* str, Gfx::Point point, Font* font, Color color) const {
	fb->draw_text(str, point, font, color);
}

void UI::DrawContext::draw_text(const char* str, Gfx::Point point, Color color) const {
	fb->draw_text(str, point, Theme::font(), color);
}

void UI::DrawContext::draw_glyph(Font* font, uint32_t codepoint, Gfx::Point pos, Color color) const {
	fb->draw_glyph(font, codepoint, pos, color);
}

void UI::DrawContext::draw_image(Ptr<const Image> img, Gfx::Point pos) const {
	img->draw(*fb, pos);
}

void UI::DrawContext::draw_image(Ptr<const Image> img, Gfx::Rect pos) const {
	img->draw(*fb, pos);
}

void UI::DrawContext::draw_image(const std::string& name, Gfx::Point pos) const {
	draw_image(Theme::image(name), pos);
}

void UI::DrawContext::draw_inset_rect(Gfx::Rect rect, Color bg, Color shadow_1, Color shadow_2, Color highlight) const {
	//Background
	fb->fill({rect.x, rect.y, rect.width, rect.height}, bg);

	//Shadow
	fb->fill({rect.x, rect.y, rect.width, 1}, shadow_1);
	fb->fill({rect.x, rect.y + 1, 1, rect.height - 1}, shadow_1);
	fb->fill({rect.x + 1, rect.y + 1, rect.width - 2, 1}, shadow_2);
	fb->fill({rect.x + 1, rect.y + 2, 1, rect.height - 3}, shadow_2);

	//Highlight
	fb->fill({rect.x + 1, rect.y + rect.height - 1, rect.width - 2, 1}, highlight);
	fb->fill({rect.x + rect.width - 1, rect.y + 1, 1, rect.height - 1}, highlight);
}

void UI::DrawContext::draw_inset_rect(Gfx::Rect rect, Color bg) const {
	draw_inset_rect(rect, bg, Theme::shadow_1(), Theme::shadow_2(), Theme::highlight());
}

void UI::DrawContext::draw_inset_rect(Gfx::Rect rect) const {
	draw_inset_rect(rect, Theme::bg(), Theme::shadow_1(), Theme::shadow_2(), Theme::highlight());
}

void UI::DrawContext::draw_outset_rect(Gfx::Rect rect, Color bg, Color shadow_1, Color shadow_2, Color highlight) const {
	//Background
	fb->fill({rect.x, rect.y, rect.width, rect.height}, bg);

	//Shadow
	fb->fill({rect.x, rect.y + rect.height - 1, rect.width - 1, 1}, shadow_2);
	fb->fill({rect.x + rect.width - 1, rect.y, 1, rect.height}, shadow_2);
	fb->fill({rect.x + 1, rect.y + rect.height - 2, rect.width - 3, 1}, shadow_1);
	fb->fill({rect.x + rect.width - 2, rect.y + 1, 1, rect.height - 2}, shadow_1);

	//Highlight
	fb->fill({rect.x, rect.y, rect.width - 1, 1}, highlight);
	fb->fill({rect.x, rect.y + 1, 1, rect.height - 2}, highlight);
}

void UI::DrawContext::draw_outset_rect(Gfx::Rect rect, Color bg) const {
	draw_outset_rect(rect, bg, Theme::shadow_1(), Theme::shadow_2(), Theme::highlight());
}

void UI::DrawContext::draw_outset_rect(Gfx::Rect rect) const {
	draw_outset_rect(rect, Theme::bg(), Theme::shadow_1(), Theme::shadow_2(), Theme::highlight());
}

void UI::DrawContext::draw_button_base(Gfx::Rect button, bool pressed) const {
	if(pressed) {
		draw_inset_rect(button, Theme::button());
	} else {
		draw_outset_rect(button, Theme::button());
	}
}

void UI::DrawContext::draw_button(Gfx::Rect rect, const std::string& text, bool pressed) const {
	draw_button_base(rect, pressed);
	int padding = Theme::button_padding() / 2 - (Theme::button_padding() % 2 == 1 ? 2 : 1 ) + (pressed ? 1 : 0);
	auto dims =  Gfx::Dimensions { Theme::font()->size_of(text.c_str()).width, Theme::font()->bounding_box().height };
	Gfx::Point text_pos = {rect.width / 2 - dims.width / 2 + padding, rect.height / 2 - dims.height / 2 + padding};
	fb->draw_text(text.c_str(), text_pos, Theme::font(), Theme::button_text());
}

void UI::DrawContext::draw_button(Gfx::Rect rect, const Framebuffer& img, bool pressed) const {
	draw_button_base(rect, pressed);
	fb->draw_image(img, rect.position() + Gfx::Point {rect.dimensions().width / 2, rect.dimensions().height / 2} - Gfx::Point {img.width / 2, img.height / 2});
}

void UI::DrawContext::draw_button(Gfx::Rect rect, Duck::Ptr<const Image> img, bool pressed) const {
	draw_button_base(rect, pressed);
	img->draw(*fb, rect.position() + Gfx::Point {rect.dimensions().width / 2, rect.dimensions().height / 2} - Gfx::Point {img->size().width / 2, img->size().height / 2});
}

void UI::DrawContext::draw_vertical_scrollbar(Gfx::Rect area, Gfx::Rect handle_area, bool enabled) const {
	fb->fill(area, UI::Theme::color("scrollbar-bg"));
	draw_outset_rect(handle_area, enabled ? UI::Theme::color("scrollbar-handle") : UI::Theme::color("scrollbar-handle-disabled"));
}

void UI::DrawContext::draw_progressbar(Gfx::Rect area, double progress) const {
	draw_inset_rect(area);
	fb->fill({
		area.x + 2,
		area.y + 2,
		(int)((area.width - 4) * progress),
		area.height - 3
	}, UI::Theme::accent());
}
