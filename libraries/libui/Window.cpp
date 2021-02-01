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

#include "Window.h"
#include "libui.h"
using namespace UI;

Window::Window() {
	_window = pond_context->create_window(nullptr, -1, -1, 1, 1);
	UI::__register_window(this, _window->id);
	_window->set_global_mouse(true);
}

Window* Window::create() {
	return new Window();
}

void Window::resize(int width, int height) {
	_window->resize(width, height);
	repaint();
}

int Window::width() {
	return _window->width;
}

int Window::height() {
	return _window->height;
}

void Window::set_position(int x, int y) {
	_window->set_position(x, y);
}

int Window::x_position() {
	return _window->x;
}

int Window::y_position() {
	return _window->y;
}


void Window::set_contents(Widget* contents) {
	_contents = contents;
	_contents->set_window(this);
	_contents->_window->set_position(DECO_LEFT_SIZE, DECO_TOP_SIZE);
	Dimensions contents_size = _contents->current_size();
	_window->resize(DECO_LEFT_SIZE + DECO_RIGHT_SIZE + contents_size.width, DECO_TOP_SIZE + DECO_BOTTOM_SIZE + contents_size.height);
	repaint();
}

Widget* Window::contents() {
	return _contents;
}

void Window::set_title(const std::string& title) {
	_title = title;
	_window->set_title(title.c_str());
}

std::string Window::title() {
	return _title;
}

void Window::repaint() {
	auto& framebuffer = _window->framebuffer;
	framebuffer.fill({0, 0, _window->width, DECO_TOP_SIZE}, RGB(50,50,50));
	framebuffer.fill({0, _window->height - DECO_BOTTOM_SIZE, _window->width, DECO_BOTTOM_SIZE}, RGB(50,50,50));
	framebuffer.fill({0, 0, DECO_LEFT_SIZE, _window->height}, RGB(50,50,50));
	framebuffer.fill({_window->width - DECO_RIGHT_SIZE, 0, DECO_RIGHT_SIZE, _window->height}, RGB(50,50,50));

	Font* title_font = pond_context->get_font("gohu-11");
	if(title_font)
		framebuffer.draw_text(_title.c_str(), {1 + DECO_LEFT_SIZE, 1}, title_font, RGB(255,255,255));

	_window->invalidate();
}

Pond::Window* Window::pond_window() {
	return _window;
}

void Window::on_keyboard(Pond::KeyEvent evt) {
	if(_contents)
		_contents->on_keyboard(evt);
}

void Window::on_mouse_move(Pond::MouseMoveEvent evt) {
	if(evt.new_x >= 0 && evt.new_x < width() && evt.new_y >= 0 && evt.new_y < height())
		mouse_in_window = true;
	else
		mouse_in_window = false;
	if(dragging)
		_window->set_position(evt.delta_x + _window->x, evt.delta_y + _window->y);
}

void Window::on_mouse_button(Pond::MouseButtonEvent evt) {
	if(mouse_in_window && !(evt.old_buttons & POND_MOUSE1) && evt.new_buttons & POND_MOUSE1) {
		dragging = true;
		drag_start = {evt.window->mouse_x, evt.window->mouse_y};
	} else if(!(evt.new_buttons & POND_MOUSE1)) {
		dragging = false;
	}
}

void Window::on_mouse_leave(Pond::MouseLeaveEvent evt) {
}
