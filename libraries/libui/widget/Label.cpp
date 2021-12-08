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

#include "Label.h"
#include <libgraphics/Font.h>

using namespace UI;
using namespace Gfx;

Label::Label(const std::string& label): _label(label) {
	_color = Theme::fg();
	_font = Theme::font();
	set_uses_alpha(true);
}

std::string Label::label() {
	return _label;
}

void Label::set_label(const std::string& new_label) {
	_label = new_label;
	update_layout();
}

Color Label::color() {
	return _color;
}

void Label::set_color(Color new_color) {
	_color = new_color;
	repaint();
}

TextAlignment Label::vertical_alignment() {
	return _v_alignment;
}

TextAlignment Label::horizontal_alignment() {
	return _h_alignment;
}

void Label::set_alignment(TextAlignment vertical, TextAlignment horizontal) {
	_v_alignment = vertical;
	_h_alignment = horizontal;
	repaint();
}

Font* Label::font() {
	return _font;
}

void Label::set_font(Font *font) {
	_font = font;
	update_layout();
}

Gfx::Dimensions Label::padding() {
	return _padding;
}

void Label::set_padding(const Gfx::Dimensions& padding) {
	_padding = padding;
	update_layout();
}

Gfx::Dimensions Label::preferred_size() {
	Gfx::Dimensions ret = _font->size_of(_label.c_str());
	ret.width += _padding.width * 2;
	ret.height += _padding.height * 2;
	return ret;
}

void Label::do_repaint(const DrawContext& ctx) {
	Gfx::Dimensions size = current_size();
	ctx.fill({0, 0, ctx.width(), ctx.height()}, RGBA(0,0,0,0));
	Gfx::Rect text_rect = {
			_padding.width,
			_padding.height,
			size.width - _padding.width * 2,
			size.height - _padding.height * 2
	};
	ctx.draw_text(_label.c_str(), text_rect, _h_alignment, _v_alignment, _font, _color);
}
