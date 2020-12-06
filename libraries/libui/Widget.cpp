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
#include "libui.h"

using namespace UI;

Dimensions Widget::preferred_size() {
	return {1, 1};
}

void Widget::repaint() {
	if(_window) {
		do_repaint(_window->framebuffer);
		_window->invalidate();
	}
}

void Widget::set_window(UI::Window* window) {
	if(_parent || _parent_window)
		return;

	Dimensions size = preferred_size();
	_window = pond_context->create_window(window->_window, 0, 0, size.width, size.height);
	__register_widget(this, _window->id);
	repaint();
	for(auto& child : children)
		child->parent_window_created();
}

void Widget::set_parent(UI::Widget* widget) {
	if(_parent || _parent_window)
		return;

	if(widget->_window) {
		Dimensions size = preferred_size();
		_window = pond_context->create_window(widget->_window, 0, 0, size.width, size.height);
		__register_widget(this, _window->id);
		repaint();
		for(auto& child : children)
			child->parent_window_created();
	}
}

void Widget::do_repaint(Image& framebuffer) {

}

void Widget::parent_window_created() {
	Dimensions size = preferred_size();
	_window = pond_context->create_window(_parent ? _parent->_window : _parent_window->_window, 0, 0, size.width, size.height);
	__register_widget(this, _window->id);
	repaint();
	for(auto& child : children)
		child->parent_window_created();
}