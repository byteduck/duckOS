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
#include "Theme.h"

using namespace UI;

Window::Window() {
	_window = pond_context->create_window(nullptr, -1, -1, 1, 1);
	UI::__register_window(this, _window->id);
	_window->set_draggable(true);
}

Window* Window::create() {
	return new Window();
}

void Window::resize(int width, int height) {
	int top_size = Theme::value("deco-top-size");
	int bottom_size = Theme::value("deco-bottom-size");
	int left_size = Theme::value("deco-left-size");
	int right_size = Theme::value("deco-right-size");
	_window->resize(left_size + right_size + width, top_size + bottom_size + height);
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
	int top_size = Theme::value("deco-top-size");
	int bottom_size = Theme::value("deco-bottom-size");
	int left_size = Theme::value("deco-left-size");
	int right_size = Theme::value("deco-right-size");
	_contents->_window->set_position(left_size, top_size);
	Dimensions contents_size = _contents->current_size();
	_window->resize(left_size + right_size + contents_size.width, top_size + bottom_size + contents_size.height);
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
	int top_size = Theme::value("deco-top-size");
	int bottom_size = Theme::value("deco-bottom-size");
	int left_size = Theme::value("deco-left-size");
	int right_size = Theme::value("deco-right-size");
	uint32_t color = Theme::color("window");
	framebuffer.fill({0, 0, _window->width, top_size}, color);
	framebuffer.fill({0, _window->height - bottom_size, _window->width, bottom_size}, color);
	framebuffer.fill({0, 0, left_size, _window->height}, color);
	framebuffer.fill({_window->width - right_size, 0, right_size, _window->height}, color);

	framebuffer.draw_image(Theme::current()->get_image("win-close"), {_window->width - 11 - right_size, 1});

	Font* title_font = Theme::font();
	if(title_font)
		framebuffer.draw_text(_title.c_str(), {1 + left_size, 1}, title_font, Theme::color("window-title"));

	_window->invalidate();
}

void Window::close() {
	_window->destroy();
}

Pond::Window* Window::pond_window() {
	return _window;
}

void Window::on_keyboard(Pond::KeyEvent evt) {
	if(_contents)
		_contents->on_keyboard(evt);
}

void Window::on_mouse_move(Pond::MouseMoveEvent evt) {
	_mouse = {evt.new_x, evt.new_y};
}

void Window::on_mouse_button(Pond::MouseButtonEvent evt) {
	if(!(evt.old_buttons & POND_MOUSE1) && (evt.new_buttons & POND_MOUSE1)) {
		int right_size = Theme::value("deco-right-size");
		if(_mouse.in({_window->width - 11 - right_size, 1, 10, 10})) {
			close();
		}
	}
}

void Window::on_mouse_leave(Pond::MouseLeaveEvent evt) {
}