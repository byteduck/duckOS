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

#include "DecorationWindow.h"
#include "Mouse.h"
#include "Display.h"

DecorationWindow::DecorationWindow(Window* parent, const Rect& contents_rect): Window(parent, calculate_decoration_rect(contents_rect)) {
	_contents = new Window(this, {DECO_LEFT_SIZE, DECO_TOP_SIZE, contents_rect.width, contents_rect.height});
	_framebuffer.fill({0, 0, _rect.width, _rect.height}, {255, 255, 255});
	set_global_mouse(true);
}

bool DecorationWindow::is_decoration() const {
	return true;
}

void DecorationWindow::mouse_moved(Point relative_pos, int delta_x, int delta_y) {
	if(dragging) {
		if(drag_start == Point{-1, -1})
			drag_start = relative_pos;
		else
			set_position(relative_pos + _absolute_rect.position() - drag_start);
	}
	Window::mouse_moved(relative_pos, delta_x, delta_y);
}

void DecorationWindow::set_mouse_buttons(uint8_t buttons) {
	if(_mouse_position.in({0, 0, _rect.width, _rect.height}))
		dragging = buttons & MOUSE_1;
	else if(!(buttons & MOUSE_1)) {
		dragging = false;
		drag_start = {-1, -1};
	}
	Window::set_mouse_buttons(buttons);
}

Rect DecorationWindow::calculate_decoration_rect(const Rect& contents_rect) {
	Rect ret = contents_rect.transform({-DECO_LEFT_SIZE, -DECO_TOP_SIZE});
	ret.width += DECO_LEFT_SIZE + DECO_RIGHT_SIZE;
	ret.height += DECO_TOP_SIZE + DECO_BOTTOM_SIZE;
	return ret;
}

Window* DecorationWindow::contents() {
	return _contents;
}

void DecorationWindow::set_content_dimensions(const Dimensions& dimensions) {
	_contents->set_dimensions(dimensions);
	set_dimensions({dimensions.width + DECO_LEFT_SIZE + DECO_RIGHT_SIZE, dimensions.height + DECO_TOP_SIZE + DECO_BOTTOM_SIZE});
}
