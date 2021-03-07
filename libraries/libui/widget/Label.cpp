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

#include "Label.h"
#include <libgraphics/font.h>

using namespace UI;

Label::Label(const std::string& label): _label(label) {
	_color = Theme::fg();
}

std::string Label::label() {
	return _label;
}

void Label::set_label(const std::string& new_label) {
	_label = new_label;
	update_size();
}

Color Label::color() {
	return _color;
}

void Label::set_color(Color new_color) {
	_color = new_color;
}

Dimensions Label::preferred_size() {
	return Theme::font()->size_of(_label.c_str());
}

void Label::do_repaint(const DrawContext& ctx) {
	ctx.draw_text(_label.c_str(), {0, 0}, _color);
}
