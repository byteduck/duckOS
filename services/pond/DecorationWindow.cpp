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
#include "FontManager.h"

DecorationWindow::DecorationWindow(Window* parent, const Rect& contents_rect): Window(parent, calculate_decoration_rect(contents_rect)) {
	_contents = new Window(this, {DECO_LEFT_SIZE, DECO_TOP_SIZE, contents_rect.width, contents_rect.height});
	redraw_frame();
}

bool DecorationWindow::is_decoration() const {
	return true;
}

void DecorationWindow::mouse_moved(Point relative_pos, int delta_x, int delta_y) {
	Window::mouse_moved(relative_pos, delta_x, delta_y);
	if(dragging) {
		set_position((relative_pos - drag_start) + _absolute_rect.position());
	}
}

void DecorationWindow::set_mouse_buttons(uint8_t buttons) {
	if(buttons == _mouse_buttons)
		return;
	_mouse_buttons = buttons;
	if(_mouse_position.in({0, 0, _rect.width, _rect.height})) {
		focus();
		move_to_front();
		if(!_mouse_position.in(_contents->_rect)) {
			dragging = buttons & MOUSE_1;
			if(dragging) {
				drag_start = _mouse_position;
				set_global_mouse(true);
			}
		}
	} else if(!(buttons & MOUSE_1)) {
		dragging = false;
		set_global_mouse(false);
	}
}

void DecorationWindow::handle_keyboard_event(const KeyboardEvent& event) {
	if(event.key == 0x3E && event.modifiers & KBD_MOD_ALT && KBD_ISPRESSED(event)) {
		//TODO: ALT+F4
		return;
	}
	_contents->handle_keyboard_event(event);
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

void DecorationWindow::redraw_frame() {
	_framebuffer.fill({0, 0, _rect.width, _rect.height}, RGBA(0,0,0,0));
	_framebuffer.fill({0, 0, _rect.width, DECO_TOP_SIZE}, RGB(50,50,50));
	_framebuffer.fill({0, _rect.height - DECO_BOTTOM_SIZE, _rect.width, DECO_BOTTOM_SIZE}, RGB(50,50,50));
	_framebuffer.fill({0, 0, DECO_LEFT_SIZE, _rect.height}, RGB(50,50,50));
	_framebuffer.fill({_rect.width - DECO_RIGHT_SIZE, 0, DECO_RIGHT_SIZE, _rect.height}, RGB(50,50,50));

	Font* title_font = FontManager::inst().get_font("gohu-11");
	if(title_font)
		_framebuffer.draw_text(_contents->title(), {DECO_LEFT_SIZE, 1}, title_font, RGB(255,255,255));

	invalidate();
}

