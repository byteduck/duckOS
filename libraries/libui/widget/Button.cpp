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

#include "Button.h"
#include "libui/libui.h"
#include "libui/Theme.h"
#include <libgraphics/font.h>

using namespace UI;

Button::Button(const std::string& label): _label(label) {

}

std::string Button::label() {
	return _label;
}

void Button::set_label(const std::string& new_label) {
	_label = new_label;
	repaint();
}

bool Button::on_mouse_button(Pond::MouseButtonEvent evt) {
	if(!(evt.old_buttons & POND_MOUSE1) && evt.window->mouse_buttons() & POND_MOUSE1) {
		_pressed = true;
		if(on_pressed)
			on_pressed();
		repaint();
		return true;
	} else if(evt.old_buttons & POND_MOUSE1 && !(evt.window->mouse_buttons() & POND_MOUSE1)){
		_pressed = false;
		if(on_released)
			on_released();
		repaint();
		return true;
	}

	return false;
}

void Button::on_mouse_leave(Pond::MouseLeaveEvent evt) {
	if(_pressed) {
		_pressed = false;
		if(on_released)
			on_released();
		repaint();
	}
}

Dimensions Button::preferred_size() {
	auto dims =  Dimensions { Theme::font()->size_of(_label.c_str()).width, Theme::font()->bounding_box().height };
	int padding = Theme::button_padding() * 2;
	dims.width += padding;
	dims.height += padding;
	return dims;
}

void Button::do_repaint(const DrawContext& ctx) {
	ctx.draw_button({0, 0, ctx.width(), ctx.height()}, _label, _pressed);
}