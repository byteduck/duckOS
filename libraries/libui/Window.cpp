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

Window::Window(): _window(pond_context->create_window(nullptr, {-1, -1, -1, -1}, true)) {
	UI::__register_window(this, _window->id());
	_window->set_draggable(true);
}

Window* Window::create() {
	return new Window();
}

void Window::resize(Dimensions dims) {
	if(_decorated) {
		_window->resize({
			UI_WINDOW_BORDER_SIZE * 2 + UI_WINDOW_PADDING * 2 + dims.width,
			UI_WINDOW_BORDER_SIZE * 2 + UI_TITLEBAR_HEIGHT + UI_WINDOW_PADDING * 3 + dims.height
		});
	} else {
		_window->resize({dims.width, dims.height});
	}
	repaint();
}

Dimensions Window::dimensions() {
	return _window->dimensions();
}

void Window::set_position(Point pos) {
	_window->set_position(pos);
}

Point Window::position() {
	return _window->position();
}

void Window::set_contents(Widget* contents) {
	_contents = contents;
	_contents->set_window(this);
	_contents->_window->set_position(UI_WINDOW_BORDER_SIZE + UI_WINDOW_PADDING, UI_WINDOW_BORDER_SIZE + UI_TITLEBAR_HEIGHT + UI_WINDOW_PADDING * 2);
	resize(_contents->current_size());
}

Widget* Window::contents() {
	return _contents;
}

void Window::set_title(const std::string& title) {
	_title = title;
	_window->set_title(title.c_str());
	repaint();
}

std::string Window::title() {
	return _title;
}

void Window::bring_to_front() {
	_window->bring_to_front();
}

void Window::repaint() {
	auto framebuffer = _window->framebuffer();
	auto ctx = DrawContext(framebuffer);
	if(_decorated) {
		Color color = Theme::window();

		//Window background/border
		ctx.draw_outset_rect({0, 0, ctx.width(), ctx.height()}, color);

		//Title bar
		Rect titlebar_rect = {
				UI_WINDOW_BORDER_SIZE + UI_WINDOW_PADDING,
				UI_WINDOW_BORDER_SIZE + UI_WINDOW_PADDING,
				ctx.width() - UI_WINDOW_BORDER_SIZE * 2 - UI_WINDOW_PADDING * 2,
				UI_TITLEBAR_HEIGHT
		};

		ctx.fill_gradient_h(titlebar_rect, Theme::window_titlebar_a(), Theme::window_titlebar_b());
		int font_height = Theme::font()->bounding_box().height;
		Point title_pos = titlebar_rect.position() + Point{4, titlebar_rect.height / 2 - font_height / 2};
		ctx.draw_text(_title.c_str(), title_pos, Theme::window_title());

		//Buttons
		int button_size = titlebar_rect.height - 4;
		_close_button.area = {
				titlebar_rect.x + titlebar_rect.width - UI_WINDOW_PADDING - button_size,
				titlebar_rect.y + 2,
				button_size,
				button_size
		};
		ctx.draw_button(_close_button.area, Theme::image(_close_button.image), _close_button.pressed);
	} else {
		ctx.fill({0, 0, ctx.width(), ctx.height()}, RGBA(0, 0, 0, 0));
	}

	_window->invalidate();
}

void Window::close() {
	_window->destroy();
}

void Window::show() {
	_window->set_hidden(false);
}

void Window::hide() {
	_window->set_hidden(true);
}

void Window::set_uses_alpha(bool uses_alpha) {
	_uses_alpha = uses_alpha;
	if(_decorated)
		_window->set_uses_alpha(uses_alpha);
}

void Window::set_decorated(bool decorated) {
	_decorated = decorated;
	update_contents_position();
	_window->set_uses_alpha(_decorated ? _uses_alpha : true);
}

Pond::Window* Window::pond_window() {
	return _window;
}

void Window::on_keyboard(Pond::KeyEvent evt) {
	if(_contents)
		_contents->on_keyboard(evt);
}

void Window::on_mouse_move(Pond::MouseMoveEvent evt) {
	_mouse = evt.new_pos;
	if(_close_button.pressed && !_mouse.in(_close_button.area)) {
		_close_button.pressed = false;
		_window->set_draggable(true);
		repaint();
	}
}

void Window::on_mouse_button(Pond::MouseButtonEvent evt) {
	if(!(evt.old_buttons & POND_MOUSE1) && (evt.new_buttons & POND_MOUSE1)) {
		if(_mouse.in(_close_button.area)) {
			_close_button.pressed = true;
			_window->set_draggable(false);
			repaint();
		}
	} else if((evt.old_buttons & POND_MOUSE1) && !(evt.new_buttons & POND_MOUSE1)) {
		if(_close_button.pressed) {
			close();
		}
	}
}

void Window::on_mouse_leave(Pond::MouseLeaveEvent evt) {
}

void Window::update_contents_position() {
	if(_decorated)
		_contents->_window->set_position(UI_WINDOW_BORDER_SIZE + UI_WINDOW_PADDING, UI_WINDOW_BORDER_SIZE + UI_TITLEBAR_HEIGHT + UI_WINDOW_PADDING * 2);
	else
		_contents->_window->set_position(0, 0);
	resize(_contents->current_size());
}
