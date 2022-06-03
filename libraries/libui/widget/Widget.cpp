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

#include "Widget.h"
#include "libui/libui.h"

using namespace UI;

Widget::~Widget() {
}

Gfx::Dimensions Widget::preferred_size() {
    if(children.size())
	    return children[0]->preferred_size();
    else
        return {1, 1};
}

Gfx::Dimensions Widget::current_size() {
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
	if(_root_window)
		_root_window->repaint();
}

void Widget::repaint_now() {
	if(_dirty && _image.data) {
		_dirty = false;
		do_repaint(_image);
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
	if(_parent_window)
		return _parent_window->shared_from_this();
	return nullptr;
}

std::shared_ptr<Window> Widget::root_window() {
	if(_root_window)
		return _root_window->shared_from_this();
	return nullptr;
}

void Widget::add_child(const std::shared_ptr<Widget>& child) {
	if(child->parent() || child->parent_window())
		return;
	children.push_back(child);
	child->set_parent(this);
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

void Widget::set_position(const Gfx::Point& position) {
	_absolute_position = position;
	update_layout();
}

void Widget::set_position_nolayout(const Gfx::Point& position) {
	_rect.set_position(position);
	recalculate_rects();
}

Gfx::Point Widget::position() {
	return _absolute_position;
}

void Widget::hide() {
	_hidden = true;
}

void Widget::show() {
	_hidden = false;
}

void Widget::set_layout_bounds(Gfx::Rect new_bounds) {
	if(new_bounds.width < 0 || new_bounds.height < 0) {
		Duck::Log::warnf("[UI] Widget dimensions {}x{} invalid!", new_bounds.width, new_bounds.height);
		new_bounds.width = std::max(new_bounds.width, 0);
		new_bounds.height = std::max(new_bounds.height, 0);
	}

	Gfx::Rect old_rect = _rect;
	_rect = new_bounds;
	_initialized_size = true;
	if(Gfx::Dimensions{_image.width, _image.height} != _rect.dimensions())
		_image = {new_bounds.width, new_bounds.height};
	recalculate_rects();
	calculate_layout();
	on_layout_change(old_rect);
	repaint();
	if(!_first_layout_done) {
		_first_layout_done = true;
		repaint_now();
	}
}

bool Widget::needs_layout_on_child_change() {
	return true;
}

void Widget::focus() {
	if(_root_window)
		_root_window->set_focused_widget(shared_from_this());
}

void Widget::set_window(Window::ArgPtr window) {
	if(_parent || _parent_window)
		return;

	_parent_window = window.get();
	set_root_window(window.get());
}

void Widget::set_root_window(Window* window) {
	_root_window = window;
	for(auto& child : children)
		child->set_root_window(window);
}

void Widget::set_parent(Widget* widget) {
	if(_parent || _parent_window)
		return;

	_parent = widget;
	set_root_window(_parent->_root_window);
	recalculate_rects();
}

void Widget::remove_parent() {
	_parent = nullptr;
	set_root_window(nullptr);
}

void Widget::update_layout() {
	//TODO: Find a better way of doing this that doesn't involve re-layouting everything everytime anything is updated
	if(_root_window)
		_root_window->calculate_layout();
}

void Widget::do_repaint(const DrawContext& framebuffer) {
	framebuffer.fill({0, 0, framebuffer.width(), framebuffer.height()}, RGBA(0, 0, 0, 0));
}

void Widget::on_child_added(const std::shared_ptr<Widget>& child) {

}

void Widget::on_child_removed(const std::shared_ptr<Widget>& child) {

}

void Widget::on_layout_change(const Gfx::Rect& old_rect) {

}

void Widget::set_uses_alpha(bool uses_alpha) {
	if(uses_alpha != _uses_alpha && _root_window)
		_root_window->repaint();
	_uses_alpha = uses_alpha;
}

void Widget::set_global_mouse(bool global_mouse) {
	_global_mouse = global_mouse;
}

Gfx::Point Widget::mouse_position() {
	return _mouse_pos;
}

unsigned int Widget::mouse_buttons() {
	return _mouse_buttons;
}

void Widget::calculate_layout() {
	for(auto& child : children) {
		Gfx::Rect child_rect = child->_rect;

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

void Widget::recalculate_rects() {
	//This assumes that the absolute rect of the parent has already been calculated
	if(_parent) {
		_absolute_rect = _rect.transform(_parent->_absolute_rect.position());
		_visible_rect = _parent->_visible_rect.overlapping_area(_rect).transform(_rect.position() * -1);
	} else {
		_absolute_rect = _rect;
		_visible_rect = _root_window ? _root_window->contents_rect().overlapping_area(_rect).transform(_rect.position() * -1) : _rect;
	}

	for(auto& child : children)
		child->recalculate_rects();
}

bool Widget::evt_mouse_move(Pond::MouseMoveEvent evt) {
	auto old_mouse_pos = _mouse_pos;
	_mouse_pos = evt.new_pos;

	bool did_consume = false;
	for(auto& child : children) {
		if(_mouse_pos.in(child->_rect)) {
			Pond::MouseMoveEvent child_evt = evt;
			child_evt.new_pos = evt.new_pos - child->_rect.position();
			did_consume = child->evt_mouse_move(child_evt);
			break;
		} else if(old_mouse_pos.in(child->_rect)) {
			child->evt_mouse_leave({
				PEVENT_MOUSE_LEAVE,
				old_mouse_pos - child->_rect.position(),
				evt.window
			});
		}
	}

	if(!did_consume)
		did_consume = on_mouse_move(evt);

	return did_consume;
}

bool Widget::evt_mouse_button(Pond::MouseButtonEvent evt) {
	evt.old_buttons = _mouse_buttons;
	_mouse_buttons = evt.new_buttons;

	bool did_consume = false;
	for(auto& child : children) {
		if(_mouse_pos.in(child->_rect)) {
			did_consume = child->evt_mouse_button(evt);
			break;
		}
	}

	if(!did_consume) {
		did_consume = on_mouse_button(evt);
		if(did_consume && !(evt.old_buttons & POND_MOUSE1) && (evt.new_buttons & POND_MOUSE1))
			focus();
	}

	return did_consume;
}

bool Widget::evt_mouse_scroll(Pond::MouseScrollEvent evt) {
	bool did_consume = false;
	for(auto& child : children) {
		if(_mouse_pos.in(child->_rect)) {
			did_consume = child->evt_mouse_scroll(evt);
			break;
		}
	}

	if(!did_consume)
		did_consume = on_mouse_scroll(evt);

	return did_consume;
}

void Widget::evt_mouse_leave(Pond::MouseLeaveEvent evt) {
	for(auto& child : children) {
		if(evt.last_pos.in(child->_rect)) {
			Pond::MouseLeaveEvent child_evt = evt;
			child_evt.last_pos = evt.last_pos - child->_rect.position();
			child->evt_mouse_leave(child_evt);
			break;
		}
	}

	on_mouse_leave(evt);
}

void Widget::evt_keyboard(Pond::KeyEvent evt) {
	if(!on_keyboard(evt) && _parent)
		_parent->evt_keyboard(evt);
}