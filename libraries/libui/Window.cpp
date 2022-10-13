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

#include <sys/time.h>
#include "Window.h"
#include "libui.h"
#include "Theme.h"
#include "libui/widget/MenuWidget.h"

using namespace UI;

Window::Window(): _window(pond_context->create_window(nullptr, {-1, -1, -1, -1}, true)) {
	_window->set_draggable(true);
}

void Window::initialize() {
	UI::__register_window(self(), _window->id());
}

void Window::resize(Gfx::Dimensions dims) {
	if(_decorated) {
		_window->resize({
			UI_WINDOW_BORDER_SIZE * 2 + UI_WINDOW_PADDING * 2 + dims.width,
			UI_WINDOW_BORDER_SIZE * 2 + UI_TITLEBAR_HEIGHT + UI_WINDOW_PADDING * 3 + dims.height
		});
	} else {
		_window->resize({dims.width, dims.height});
	}
	if(_contents)
		_contents->update_layout();
	repaint();
}

Gfx::Dimensions Window::dimensions() {
	return _window->dimensions();
}

Gfx::Rect Window::contents_rect() {
	if(_decorated) {
		Gfx::Rect ret = _window->rect();
		ret.width -= UI_WINDOW_BORDER_SIZE * 2 + UI_WINDOW_PADDING * 2;
		ret.height -= UI_WINDOW_BORDER_SIZE * 2 + UI_TITLEBAR_HEIGHT + UI_WINDOW_PADDING * 3;
		ret.x = UI_WINDOW_PADDING + UI_WINDOW_BORDER_SIZE;
		ret.y = UI_WINDOW_PADDING * 2 + UI_WINDOW_BORDER_SIZE + UI_TITLEBAR_HEIGHT;
		return ret;
	} else {
		Gfx::Dimensions dims = _window->dimensions();
		return {0, 0, dims.width, dims.height};
	}
}

void Window::set_position(Gfx::Point pos) {
	_window->set_position(pos);
}

Gfx::Point Window::position() {
	return _window->position();
}

void Window::set_contents(const std::shared_ptr<Widget>& contents) {
	_contents = contents;
	_focused_widget = contents;
	_contents->set_window(self());
	resize(_contents->current_size());
}

std::shared_ptr<Widget> Window::contents() {
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

void Window::set_resizable(bool resizable) {
	_resizable = resizable;
	_window->set_resizable(resizable);
}

bool Window::resizable() {
	return _resizable;
}

bool Window::is_focused() {
	return _focused;
}

void Window::bring_to_front() {
	_window->bring_to_front();
}

void Window::repaint() {
	_needs_repaint = true;
}

void Window::repaint_now() {
	if(!_needs_repaint)
		return;
	_needs_repaint = false;

	//Next, draw the window frame
	auto framebuffer = _window->framebuffer();
	auto ctx = DrawContext(framebuffer);
	if(_decorated) {
		Color color = Theme::window();

		//Window background/border
		ctx.draw_outset_rect({0, 0, ctx.width(), ctx.height()}, color);

		//Title bar
		Gfx::Rect titlebar_rect = {
				UI_WINDOW_BORDER_SIZE + UI_WINDOW_PADDING,
				UI_WINDOW_BORDER_SIZE + UI_WINDOW_PADDING,
				ctx.width() - UI_WINDOW_BORDER_SIZE * 2 - UI_WINDOW_PADDING * 2,
				UI_TITLEBAR_HEIGHT
		};

		//Title bar background
		ctx.fill_gradient_h(titlebar_rect, Theme::window_titlebar_a(), Theme::window_titlebar_b());

		//Title bar icon
		int title_xpos = 4;
		if(UI::app_info().exists()) {
			auto& icon = UI::app_info().icon();
			ctx.draw_image(icon, titlebar_rect.position() + Gfx::Point {2, titlebar_rect.height / 2 - icon.height / 2});
			title_xpos += 2 + icon.width;
		}

		//Title bar text
		int font_height = Theme::font()->bounding_box().height;
		Gfx::Point title_pos = titlebar_rect.position() + Gfx::Point{title_xpos, titlebar_rect.height / 2 - font_height / 2};
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

	//Then, draw widgets
	if(_contents)
		blit_widget(_contents);
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

void Window::resize_to_contents() {
	Gfx::Dimensions contents_size = _contents->preferred_size();
	resize(contents_size);
}

void Window::set_uses_alpha(bool uses_alpha) {
	_uses_alpha = uses_alpha;
	if(_decorated)
		_window->set_uses_alpha(uses_alpha);
}

void Window::set_decorated(bool decorated) {
	if(decorated == _decorated)
		return;
	_decorated = decorated;
	_window->set_uses_alpha(_decorated ? _uses_alpha : true);

	//Adjust the rect of the window to keep the contents in the same position
	Gfx::Rect new_rect = _window->rect();
	if(decorated) {
		new_rect.x -= UI_WINDOW_BORDER_SIZE + UI_WINDOW_PADDING;
		new_rect.y -= UI_WINDOW_BORDER_SIZE + UI_TITLEBAR_HEIGHT + UI_WINDOW_PADDING * 2;
		new_rect.width += UI_WINDOW_BORDER_SIZE * 2 + UI_WINDOW_PADDING * 2;
		new_rect.height += UI_WINDOW_BORDER_SIZE * 2 + UI_TITLEBAR_HEIGHT + UI_WINDOW_PADDING * 3;
	} else {
		new_rect.x += UI_WINDOW_BORDER_SIZE + UI_WINDOW_PADDING;
		new_rect.y += UI_WINDOW_BORDER_SIZE + UI_TITLEBAR_HEIGHT + UI_WINDOW_PADDING * 2;
		new_rect.width -= UI_WINDOW_BORDER_SIZE * 2 + UI_WINDOW_PADDING * 2;
		new_rect.height -= UI_WINDOW_BORDER_SIZE * 2 + UI_TITLEBAR_HEIGHT + UI_WINDOW_PADDING * 3;
	}
	_window->set_position(new_rect.position());
	_window->resize(new_rect.dimensions());
	repaint();

	//Update contents positioning
	if(_contents)
		_contents->update_layout();
}

Pond::Window* Window::pond_window() {
	return _window;
}

void Window::on_keyboard(Pond::KeyEvent evt) {
	if(_focused_widget)
		_focused_widget->on_keyboard(evt);
}

void Window::on_mouse_move(Pond::MouseMoveEvent evt) {
	//TODO: Global mouse events

	_abs_mouse = evt.abs_pos;
	auto old_mouse = _mouse;
	_mouse = evt.new_pos;

	if(_close_button.pressed && !_mouse.in(_close_button.area)) {
		_close_button.pressed = false;
		_window->set_draggable(true);
		repaint();
	}

	//TODO: Add some mechanism in pond to exclude an area from dragging
	if(!_mouse.in(contents_rect()) && old_mouse.in(contents_rect()))
		_window->set_draggable(true);
	else if(_mouse.in(contents_rect()) && old_mouse.in(contents_rect()))
		_window->set_draggable(false);

	if(_contents && evt.new_pos.in(_contents->_rect)) {
		evt.new_pos = evt.new_pos - _contents->_rect.position();
		_contents->evt_mouse_move(evt);
	} else if(_contents && old_mouse.in(_contents->_rect)) {
		_contents->evt_mouse_leave({
			PEVENT_MOUSE_LEAVE,
			old_mouse - _contents->_rect.position(),
			evt.window
		});
	}
}

void Window::on_mouse_button(Pond::MouseButtonEvent evt) {
	if(!(evt.old_buttons & POND_MOUSE1) && (evt.new_buttons & POND_MOUSE1)) {
		_window->bring_to_front();
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

	if(_contents && _mouse.in(_contents->_rect)) {
		_contents->evt_mouse_button(evt);
	}
}

void Window::on_mouse_scroll(Pond::MouseScrollEvent evt) {
	if(_contents && _mouse.in(_contents->_rect)) {
		_contents->evt_mouse_scroll(evt);
	}
}

void Window::on_mouse_leave(Pond::MouseLeaveEvent evt) {
	if(_contents && evt.last_pos.in(_contents->_rect)) {
		evt.last_pos = evt.last_pos - _contents->_rect.position();
		_contents->evt_mouse_leave(evt);
	}
}

void Window::on_resize(const Gfx::Rect& old_rect) {
	calculate_layout();
}

void Window::on_focus(bool focused) {
	_focused = focused;
	if(delegate.lock())
		delegate.lock()->window_focus_changed(self(), focused);
}

void Window::calculate_layout() {
	if(!_contents)
		return;

	Gfx::Rect new_rect = _contents->_rect;
	Gfx::Rect bounds = contents_rect();

	switch(_contents->_sizing_mode) {
		case FILL:
			new_rect.set_dimensions(bounds.dimensions());
			break;
		case PREFERRED:
			new_rect.set_dimensions(_contents->preferred_size());
			break;
	}

	switch(contents()->_positioning_mode) {
		case AUTO: {
			auto dims = new_rect.dimensions();
			new_rect.set_position({
				bounds.x + bounds.width / 2 - dims.width / 2,
				bounds.y + bounds.height / 2 - dims.height / 2
			});
			break;
		}

		case ABSOLUTE: {
			new_rect.set_position(bounds.position() + _contents->_absolute_position);
			break;
		}
	}

	_contents->set_layout_bounds(new_rect);
}

void Window::blit_widget(PtrRef<Widget> widget) {
	if(widget->_hidden)
		return;

	widget->repaint_now();
	Gfx::Point widget_pos = widget->_absolute_rect.position() + widget->_visible_rect.position();
	if(widget->_uses_alpha)
		_window->framebuffer().copy_blitting(widget->_image, widget->_visible_rect, widget_pos);
	else
		_window->framebuffer().copy(widget->_image, widget->_visible_rect, widget_pos);
	
	for(auto& child : widget->children)
		blit_widget(child);
}

void Window::open_menu(std::shared_ptr<Menu>& menu) {
	open_menu(menu, _mouse);
}

void Window::open_menu(std::shared_ptr<Menu>& menu, Gfx::Point point) {
	MenuWidget::open_menu(menu, point + _window->position());
}

void Window::set_focused_widget(PtrRef<Widget> widget) {
	_focused_widget = widget;
}