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
	for(int x = 0; x < rect.width; x++) {
		double pct = (double)x / rect.width;
		double oneminus = 1.0 - pct;
		fb->fill({rect.x + x, rect.y, 1, rect.height}, RGBA(
				(uint8_t)(COLOR_R(color_a) * oneminus + COLOR_R(color_b) * pct),
				(uint8_t)(COLOR_G(color_a) * oneminus + COLOR_G(color_b) * pct),
				(uint8_t)(COLOR_B(color_a) * oneminus + COLOR_B(color_b) * pct),
				(uint8_t)(COLOR_A(color_a) * oneminus + COLOR_A(color_b) * pct)
		));
	}
}

void UI::DrawContext::draw_text(const char* str, Gfx::Rect rect, TextAlignment h_align, TextAlignment v_align, Font* font, Color color) const {
	Gfx::Point text_pos = rect.position();
	auto dims =  Gfx::Dimensions { font->size_of(str).width, font->bounding_box().height };
	switch(h_align) {
		case BEGINNING:
			break;
		case CENTER:
			text_pos.x = rect.x + rect.width / 2 - dims.width / 2;
			break;
		case END:
			text_pos.x = rect.x + rect.width - dims.width;
			break;
	}
	switch(v_align) {
		case BEGINNING:
			break;
		case CENTER:
			text_pos.y = rect.y + rect.height / 2 - dims.height / 2;
			break;
		case END:
			text_pos.y = rect.y + rect.height - dims.height;
			break;
	}
	fb->draw_text(str, text_pos, font, color);
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
