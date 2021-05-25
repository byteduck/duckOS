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

#include "Widget.h"
#include "libui/libui.h"

using namespace UI;

Dimensions Widget::preferred_size() {
	return {1, 1};
}

Dimensions Widget::current_size() {
	if(!_initialized_size) {
        _rect.set_dimensions(preferred_size());
        _initialized_size = true;
    }
	return _rect.dimensions();
}

Rect Widget::bounds_for_child(Widget *child) {
    return {0, 0, _rect.width, _rect.height};
}

PositioningMode Widget::positioning_mode() {
    return _positioning_mode;
}

void Widget::set_positioning_mode(PositioningMode mode) {
    _positioning_mode = mode;
    update_layout();
}

SizingMode Widget::sizing_mode() {
    return _sizing_mode;
}

void Widget::set_sizing_mode(SizingMode mode) {
    _sizing_mode = mode;
    update_layout();
}

void Widget::repaint() {
	if(_window) {
		do_repaint(_window->framebuffer());
		_window->invalidate();
	}
}

bool Widget::on_keyboard(Pond::KeyEvent evt) {
	return false;
}

bool Widget::on_mouse_move(Pond::MouseMoveEvent evt) {
	return true;
}

bool Widget::on_mouse_button(Pond::MouseButtonEvent evt) {
	return true;
}

void Widget::on_mouse_leave(Pond::MouseLeaveEvent evt) {

}

Widget* Widget::parent() {
	return _parent;
}

Window* Widget::parent_window() {
	return _parent_window;
}

Window* Widget::root_window() {
	return _parent_window ? _parent_window : _parent->root_window();
}

void Widget::add_child(Widget* child) {
	if(child->parent() || child->parent_window())
		return;
	children.push_back(child);
	child->set_parent(this);
	on_child_added(child);
	update_layout();
}

void Widget::set_position(const Point& position) {
    _absolute_position = position;
    update_layout();
}

Point Widget::position() {
	return _absolute_position;
}

void Widget::hide() {
	_hidden = true;
	if(_window)
		_window->set_hidden(true);
}

void Widget::show() {
	_hidden = false;
	if(_window)
		_window->set_hidden(false);
}

void Widget::set_window(UI::Window* window) {
	if(_parent || _parent_window)
		return;

	_parent_window = window;
	create_window(window->_window);
}

void Widget::set_parent(UI::Widget* widget) {
	if(_parent || _parent_window)
		return;

	_parent = widget;

	if(widget->_window)
		create_window(widget->_window);
}

void Widget::update_layout() {
    if(!_parent_window && !_parent)
        return;

    Rect old_rect = _rect;
    Rect parent_rect = _parent_window ? _parent_window->contents_rect() : _parent->bounds_for_child(this);

    switch(_sizing_mode) {
        case FILL:
            _rect.set_dimensions(parent_rect.dimensions());
            break;
        case PREFERRED:
            _rect.set_dimensions(preferred_size());
            break;
    }

    switch(_positioning_mode) {
        case AUTO: {
            auto dims = _rect.dimensions();
            _rect.set_position({
                parent_rect.position().x + parent_rect.width / 2 - dims.width / 2,
                parent_rect.position().y + parent_rect.height / 2 - dims.height / 2
            });
            break;
        }
        case ABSOLUTE: {
            _rect.set_position(parent_rect.position() + _absolute_position);
            break;
        }
    }

	on_layout_change(old_rect);

    if(_window) {
        _window->set_position(_rect.position());
        _window->resize(_rect.dimensions());
        repaint();
    }

    for(auto child : children) {
        child->update_layout();
    }
}

void Widget::do_repaint(const DrawContext& framebuffer) {
	framebuffer.fill({0, 0, framebuffer.width(), framebuffer.height()}, RGBA(0, 0, 0, 0));
}

void Widget::on_child_added(UI::Widget* child) {

}

void Widget::on_layout_change(const Rect& old_rect) {

}

void Widget::set_uses_alpha(bool uses_alpha) {
	_uses_alpha = uses_alpha;
	if(_window)
		_window->set_uses_alpha(uses_alpha);
}

void Widget::set_global_mouse(bool global_mouse) {
	_global_mouse = global_mouse;
	if(_window)
		_window->set_global_mouse(_global_mouse);
}

Point Widget::mouse_position() {
	return _window ? _window->mouse_pos() : Point {0, 0};
}

unsigned int Widget::mouse_buttons() {
	return _window ? _window->mouse_buttons() : 0;
}

void Widget::parent_window_created() {
	create_window(_parent ? _parent->_window : _parent_window->_window);
}

void Widget::create_window(Pond::Window* parent) {
    _rect.set_dimensions(preferred_size());
    _window = pond_context->create_window(parent, _rect, _hidden);
    _window->set_uses_alpha(_uses_alpha);
    _window->set_global_mouse(_global_mouse);
    __register_widget(this, _window->id());
    repaint();
    for(auto &child : children)
        child->parent_window_created();
}
