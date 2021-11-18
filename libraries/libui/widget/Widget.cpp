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

Widget::~Widget() {
    destroy_window();
}

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
	_dirty = true;
}

void Widget::repaint_now() {
	if(_dirty && _window) {
		_dirty = false;
		do_repaint(_window->framebuffer());
		_window->invalidate();
	}
}

bool Widget::on_keyboard(Pond::KeyEvent evt) {
	return false;
}

bool Widget::on_mouse_move(Pond::MouseMoveEvent evt) {
	return false;
}

bool Widget::on_mouse_button(Pond::MouseButtonEvent evt) {
	return false;
}

bool Widget::on_mouse_scroll(Pond::MouseScrollEvent evt) {
	return false;
}

void Widget::on_mouse_leave(Pond::MouseLeaveEvent evt) {

}

std::shared_ptr<Widget> Widget::parent() {
    if(!_parent)
        return nullptr;
	return _parent->shared_from_this();
}

std::shared_ptr<Window> Widget::parent_window() {
	return _parent_window;
}

std::shared_ptr<Window> Widget::root_window() {
	if(!_parent_window && !_parent)
		return nullptr;
	return _parent_window ? _parent_window : _parent->root_window();
}

void Widget::add_child(const std::shared_ptr<Widget>& child) {
	if(child->parent() || child->parent_window())
		return;
	children.push_back(child);
	child->set_parent(shared_from_this());
	on_child_added(child);
    if(needs_layout_on_child_change())
	    update_layout();
}

bool Widget::remove_child(const std::shared_ptr<Widget>& child) {
    if(child->parent().get() != this)
        return false;

    auto child_it = std::find(children.begin(), children.end(), child);
    if(child_it == children.end())
        return false;

    children.erase(child_it);
    child->remove_parent();
    on_child_removed(child);
    if(needs_layout_on_child_change())
        update_layout();
    return true;
}

void Widget::set_position(const Point& position) {
    _absolute_position = position;
    update_layout();
}

void Widget::set_position_nolayout(const Point& position) {
    _rect.set_position(position);
    _window->set_position(_rect.position());
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

void Widget::set_layout_bounds(Rect new_bounds) {
	Rect old_rect = _rect;
	_rect = new_bounds;
    _initialized_size = true;
	calculate_layout();
	on_layout_change(old_rect);
	if(_window) {
		_window->set_position(_rect.position());
		_window->resize(_rect.dimensions());
		repaint();
		if(!_first_layout_done) {
			_first_layout_done = true;
			repaint_now();
			_window->set_hidden(_hidden);
		}
	}
}

bool Widget::needs_layout_on_child_change() {
    return true;
}

void Widget::set_window(const std::shared_ptr<Window>& window) {
	if(_parent || _parent_window)
		return;

	_parent_window = window;
	create_window(window->_window);
}

void Widget::set_parent(const std::shared_ptr<Widget>& widget) {
	if(_parent || _parent_window)
		return;

	_parent = widget.get();

	if(widget->_window)
		create_window(widget->_window);
}

void Widget::remove_parent() {
    _parent = nullptr;
    destroy_window();
}

void Widget::update_layout() {
	//TODO: Find a better way of doing this that doesn't involve re-layouting everything everytime anything is updated
	auto root = root_window();
	if(root)
		root->calculate_layout();
}

void Widget::do_repaint(const DrawContext& framebuffer) {
	framebuffer.fill({0, 0, framebuffer.width(), framebuffer.height()}, RGBA(0, 0, 0, 0));
}

void Widget::on_child_added(const std::shared_ptr<Widget>& child) {

}

void Widget::on_child_removed(const std::shared_ptr<Widget>& child) {

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

void Widget::calculate_layout() {
	for(auto& child : children) {
		Rect child_rect = child->_rect;

		switch(child->_sizing_mode) {
			case FILL:
				child_rect.set_dimensions(_rect.dimensions());
				break;
			case PREFERRED:
				child_rect.set_dimensions(preferred_size());
				break;
		}

		switch(child->_positioning_mode) {
			case AUTO: {
				auto dims = child_rect.dimensions();
				child_rect.set_position({
					_rect.x + _rect.width / 2 - dims.width / 2,
					_rect.y + _rect.height / 2 - dims.height / 2
				});
				break;
			}
			case ABSOLUTE: {
				child_rect.set_position(_rect.position() + child->_absolute_position);
				break;
			}
		}

		child->set_layout_bounds(child_rect);
	}
}

void Widget::parent_window_created() {
	create_window(_parent ? _parent->_window : _parent_window->_window);
}

void Widget::parent_window_destroyed() {
    if(!_window)
        return;
    __deregister_widget(_window->id());
    delete _window;
    _window = nullptr;
    for(auto child : children)
        child->parent_window_destroyed();
}

void Widget::create_window(Pond::Window* parent) {
    _rect.set_dimensions(current_size());
    _window = pond_context->create_window(parent, _rect, true);
    _window->set_uses_alpha(_uses_alpha);
    _window->set_global_mouse(_global_mouse);
    __register_widget(shared_from_this(), _window->id());
    for(auto &child : children)
        child->parent_window_created();
}

void Widget::destroy_window() {
    if(!_window)
        return;
    __deregister_widget(_window->id());
    _window->destroy();
    delete _window;
    _window = nullptr;
    for(auto child : children)
        child->parent_window_destroyed();
}
