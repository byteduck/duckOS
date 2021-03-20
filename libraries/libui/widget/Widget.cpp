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
	if(!_initialized_size)
		_rect.set_dimensions(preferred_size());
	return _rect.dimensions();
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
}

void Widget::set_position(const Point& position) {
	if(_window)
		_window->set_position(position.x, position.y);
	_rect.set_position(position);
}

Point Widget::position() {
	return _rect.position();
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

void Widget::update_size() {
	_rect.set_dimensions(preferred_size());

	if(_window) {
		_window->resize(_rect.dimensions());
		repaint();
	}

	if(_parent)
		_parent->update_size();

	if(_parent_window)
		_parent_window->resize(_rect.dimensions());
}

void Widget::do_repaint(const DrawContext& framebuffer) {

}

void Widget::on_child_added(UI::Widget* child) {

}

void Widget::set_uses_alpha(bool uses_alpha) {
	_uses_alpha = uses_alpha;
	if(_window)
		_window->set_uses_alpha(uses_alpha);
}

void Widget::parent_window_created() {
	create_window(_parent ? _parent->_window : _parent_window->_window);
}

void Widget::create_window(Pond::Window* parent) {
	_rect.set_dimensions(preferred_size());
	_window = pond_context->create_window(parent, _rect, _hidden);
	_window->set_uses_alpha(_uses_alpha);
	__register_widget(this, _window->id());
	repaint();
	for(auto& child : children)
		child->parent_window_created();
}
