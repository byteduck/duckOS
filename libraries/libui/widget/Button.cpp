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

#include "Button.h"
#include "libui/libui.h"
#include "libui/Theme.h"
#include <libgraphics/font.h>

using namespace UI;

Button::Button(std::string label): _label(std::move(label)), _is_image_button(false) {

}

Button::Button(Gfx::Image image): _image(std::move(image)), _is_image_button(true) {

}

const std::string& Button::label() {
	return _label;
}

void Button::set_label(std::string new_label) {
	_is_image_button = false;
	_label = std::move(new_label);
	repaint();
}

const Gfx::Image& Button::image() {
	return _image;
}

void Button::set_image(Gfx::Image new_image) {
	_is_image_button = true;
	_image = std::move(new_image);
	repaint();
}

bool Button::on_mouse_button(Pond::MouseButtonEvent evt) {
	if(!(evt.old_buttons & POND_MOUSE1) && (evt.new_buttons & POND_MOUSE1)) {
		_pressed = true;
		if(on_pressed)
			on_pressed();
		repaint();
		return true;
	} else if((evt.old_buttons & POND_MOUSE1) && !(evt.new_buttons & POND_MOUSE1)) {
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
	if(_is_image_button) {
		auto dims = Dimensions {_image.width, _image.height};
		int padding = 4;
		dims.width += padding;
		dims.height += padding;
		return dims;
	} else {
		auto dims = Dimensions{Theme::font()->size_of(_label.c_str()).width, Theme::font()->bounding_box().height};
		int padding = Theme::button_padding() * 2;
		dims.width += padding;
		dims.height += padding;
		return dims;
	}
}

void Button::do_repaint(const DrawContext& ctx) {
	if(_is_image_button)
		ctx.draw_button({0, 0, ctx.width(), ctx.height()}, _image, _pressed);
	else
		ctx.draw_button({0, 0, ctx.width(), ctx.height()}, _label, _pressed);
}
