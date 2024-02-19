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

#define highlighted(col) ((col).lightened(0.3))
#define shadow1(col) ((col).darkened(0.4))
#define shadow2(col) ((col).darkened(0.5))

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

void UI::DrawContext::fill(Gfx::Rect rect, Gfx::Color color) const {
	fb->fill(rect, color);
}

void UI::DrawContext::fill_gradient_h(Gfx::Rect rect, Gfx::Color color_a, Gfx::Color color_b) const {
	fb->fill_gradient_h(rect, color_a, color_b);
}

void UI::DrawContext::fill_gradient_v(Gfx::Rect rect, Gfx::Color color_a, Gfx::Color color_b) const {
	fb->fill_gradient_v(rect, color_a, color_b);
}

void UI::DrawContext::fill_ellipse(Gfx::Rect rect, Gfx::Color color) const {
	float mid_y = rect.height / 2.0f;
	float height_squared = rect.height * rect.height;
	for (int line = 0; line < rect.height; line++) {
		float y = line - mid_y;
		float radius = rect.width * sqrt(0.25f - (y * y) / height_squared);
		fill({rect.x + rect.width / 2 - (int) radius, rect.y + line, (int) (radius * 2), 1}, color);
	}
}

void UI::DrawContext::fill_rounded_rect(Gfx::Rect rect, Gfx::Color color, int radius) const {
	// Fill middle and outer bits, excluding corners
	fill(rect.inset(radius), color);
	fill(rect.inset(0, radius, rect.height - radius, radius), color);
	fill(rect.inset(rect.height - radius, radius, 0, radius), color);
	fill(rect.inset(radius, 0, radius, rect.width - radius), color);
	fill(rect.inset(radius, rect.width - radius, radius, 0), color);

	// Draw corners
	Gfx::Dimensions corner_dims = {radius * 2, radius * 2};
	fill_ellipse({rect.position(), corner_dims}, color);
	fill_ellipse({rect.position() + Point { rect.width - radius * 2, 0 }, corner_dims}, color);
	fill_ellipse({rect.position() + Point { 0, rect.height - radius * 2 }, corner_dims}, color);
	fill_ellipse({rect.position() + Point { rect.width - radius * 2, rect.height - radius * 2 }, corner_dims}, color);
}

void UI::DrawContext::draw_text(const UI::TextLayout& layout, Gfx::Rect rect, UI::TextAlignment h_align, UI::TextAlignment v_align, Gfx::Color color) const {
	// Next, apply vertical alignment
	Point text_pos = rect.position();
	switch(v_align) {
		case BEGINNING:
			break;
		case CENTER:
			text_pos.y = rect.y + rect.dimensions().height / 2 - layout.dimensions().height / 2;
			break;
		case END:
			text_pos.y = rect.y + rect.dimensions().height - layout.dimensions().height;
			break;
	}

	// Then, draw all the lines
	for(auto& line : layout.lines()) {
		if ((text_pos.y + layout.font()->bounding_box().height) >= 0 && text_pos.y < (rect.y + rect.height)) {
			switch(h_align) {
				case BEGINNING:
					break;
				case CENTER:
					text_pos.x = rect.x + rect.dimensions().width / 2 - line.bounds.width / 2;
					break;
				case END:
					text_pos.x = rect.x + rect.dimensions().width - line.bounds.width;
					break;
			}
			fb->draw_text(line.text.c_str(), text_pos, layout.font(), color);
		}
		text_pos = {rect.x, text_pos.y + layout.font()->bounding_box().height};
	}
}

void UI::DrawContext::draw_text(const char* str, Gfx::Rect rect, TextAlignment h_align, TextAlignment v_align, Font* font, Gfx::Color color, TruncationMode truncation) const {
	draw_text(TextLayout(str, rect.dimensions(), font, truncation, TextLayout::BreakMode::WORD), rect, h_align, v_align, color);
}

void UI::DrawContext::draw_text(const char* str, Gfx::Point point, Font* font, Gfx::Color color) const {
	fb->draw_text(str, point, font, color);
}

void UI::DrawContext::draw_text(const char* str, Gfx::Point point, Gfx::Color color) const {
	fb->draw_text(str, point, Theme::font(), color);
}

void UI::DrawContext::draw_glyph(Font* font, uint32_t codepoint, Gfx::Point pos, Gfx::Color color) const {
	fb->draw_glyph(font, codepoint, pos, color);
}

void UI::DrawContext::draw_image(Duck::Ptr<const Image> img, Gfx::Point pos) const {
	img->draw(*fb, pos);
}

void UI::DrawContext::draw_image(Duck::Ptr<const Image> img, Gfx::Rect pos) const {
	img->draw(*fb, pos);
}

void UI::DrawContext::draw_image(const std::string& name, Gfx::Point pos) const {
	draw_image(Theme::image(name), pos);
}

void UI::DrawContext::draw_inset_outline(Gfx::Rect rect, Gfx::Color shadow_1, Gfx::Color shadow_2, Gfx::Color highlight) const {
	//Shadow
	fb->fill({rect.x, rect.y, rect.width, 1}, shadow_1);
	fb->fill({rect.x, rect.y + 1, 1, rect.height - 1}, shadow_1);
	fb->fill({rect.x + 1, rect.y + 1, rect.width - 2, 1}, shadow_2);
	fb->fill({rect.x + 1, rect.y + 2, 1, rect.height - 3}, shadow_2);

	//Highlight
	fb->fill({rect.x + 1, rect.y + rect.height - 1, rect.width - 2, 1}, highlight);
	fb->fill({rect.x + rect.width - 1, rect.y + 1, 1, rect.height - 1}, highlight);
}

void UI::DrawContext::draw_inset_outline(Gfx::Rect rect) const {
	draw_inset_outline(rect, Theme::shadow_1(), Theme::shadow_2(), Theme::highlight());
}

void UI::DrawContext::draw_inset_rect(Gfx::Rect rect, Gfx::Color bg1, Gfx::Color bg2, Gfx::Color shadow_1, Gfx::Color shadow_2, Gfx::Color highlight) const {
	fb->fill_gradient_v(rect, bg1, bg2);
	draw_inset_outline(rect, shadow_1, shadow_2, highlight);
}

void UI::DrawContext::draw_inset_rect(Gfx::Rect rect, Gfx::Color bg, Gfx::Color shadow_1, Gfx::Color shadow_2, Gfx::Color highlight) const {
	draw_inset_rect(rect, bg, bg, shadow_1, shadow_2, highlight);
}

void UI::DrawContext::draw_inset_rect(Gfx::Rect rect, Gfx::Color bg1, Gfx::Color bg2) const {
	draw_inset_rect(rect, bg1, bg2, shadow1(bg1), shadow2(bg1), highlighted(bg1));
}

void UI::DrawContext::draw_inset_rect(Gfx::Rect rect, Gfx::Color bg) const {
	draw_inset_rect(rect, bg, shadow1(bg), shadow2(bg), highlighted(bg));
}

void UI::DrawContext::draw_inset_rect(Gfx::Rect rect) const {
	draw_inset_rect(rect, Theme::bg(), Theme::shadow_1(), Theme::shadow_2(), Theme::highlight());
}

void UI::DrawContext::draw_outset_rect(Gfx::Rect rect, Gfx::Color bg1, Gfx::Color bg2, Gfx::Color shadow_1, Gfx::Color shadow_2, Gfx::Color highlight) const {
	//Background
	if((rect.width * 2) > rect.height)
		fb->fill_gradient_v(rect, bg1, bg2);
	else
		fb->fill_gradient_h(rect, bg1, bg2);

	//Shadow
	fb->fill({rect.x, rect.y + rect.height - 1, rect.width - 1, 1}, shadow_2);
	fb->fill({rect.x + rect.width - 1, rect.y, 1, rect.height}, shadow_2);
	fb->fill({rect.x + 1, rect.y + rect.height - 2, rect.width - 3, 1}, shadow_1);
	fb->fill({rect.x + rect.width - 2, rect.y + 1, 1, rect.height - 2}, shadow_1);

	//Highlight
	fb->fill({rect.x, rect.y, rect.width - 1, 1}, highlight);
	fb->fill({rect.x, rect.y + 1, 1, rect.height - 2}, highlight);
}

void UI::DrawContext::draw_outset_rect(Gfx::Rect rect, Gfx::Color bg, Gfx::Color shadow_1, Gfx::Color shadow_2, Gfx::Color highlight) const {
	draw_outset_rect(rect, bg, bg.darkened(), shadow_1, shadow_2, highlight);
}

void UI::DrawContext::draw_outset_rect(Gfx::Rect rect, Gfx::Color bg1, Gfx::Color bg2) const {
	draw_outset_rect(rect, bg1, bg2, shadow1(bg1), shadow2(bg1), highlighted(bg1));
}

void UI::DrawContext::draw_outset_rect(Gfx::Rect rect, Gfx::Color bg) const {
	draw_outset_rect(rect, bg, shadow1(bg), shadow2(bg), highlighted(bg));
}

void UI::DrawContext::draw_outset_rect(Gfx::Rect rect) const {
	draw_outset_rect(rect, Theme::bg(), Theme::shadow_1(), Theme::shadow_2(), Theme::highlight());
}

void UI::DrawContext::draw_button_base(Gfx::Rect button, bool pressed) const {
	draw_button_base(button, pressed, Theme::button());
}

void UI::DrawContext::draw_button_base(Gfx::Rect button, bool pressed, Gfx::Color color) const {
	if(pressed) {
		draw_inset_rect(button, color);
	} else {
		draw_outset_rect(button, color);
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
	Gfx::Point img_pos = rect.position()
			+ Gfx::Point {rect.dimensions().width / 2, rect.dimensions().height / 2}
			- Gfx::Point {img->size().width / 2, img->size().height / 2}
			+ (pressed ? Gfx::Point { 1, 1 } : Gfx::Point { 0, 0 });
	img->draw(*fb, img_pos);
}

void UI::DrawContext::draw_vertical_scrollbar(Gfx::Rect area, Gfx::Rect handle_area, bool enabled) const {
	fb->fill(area, Theme::scrollbar_bg());
	auto handle_color = enabled ? Theme::scrollbar_handle() : Theme::scrollbar_handle_disabled();
	auto notch_color = handle_color.darkened(0.5);
	draw_outset_rect(handle_area, handle_color);
	if (enabled) {
		fill({handle_area.x + 2, handle_area.y + handle_area.height / 2 - 2, handle_area.width - 5, 1}, notch_color);
		fill({handle_area.x + 2, handle_area.y + handle_area.height / 2, handle_area.width - 5, 1}, notch_color);
		fill({handle_area.x + 2, handle_area.y + handle_area.height / 2 + 2, handle_area.width - 5, 1}, notch_color);
	}
}

void UI::DrawContext::draw_progressbar(Gfx::Rect area, double progress) const {
	draw_inset_rect(area);
	if(progress != 0) {
		draw_outset_rect({
			area.x + 2,
			area.y + 2,
			(int) ((area.width - 4) * progress),
			area.height - 3
		}, UI::Theme::accent());
	}
}
