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
#include <cstdio>
#include "Display.h"
#include <memory.h>
#include "DecorationWindow.h"

int Window::current_id = 0;

Window::Window(Window* parent, const Rect& rect): _parent(parent), _rect(rect), _display(parent->_display), _id(++current_id) {
	alloc_framebuffer();
	_parent->_children.push_back(this);
	_display->add_window(this);
	recalculate_rects();
	invalidate();
}

Window::Window(Display* display): _parent(nullptr), _rect(display->dimensions()), _display(display), _id(++current_id) {
	alloc_framebuffer();
	_display->set_root_window(this);
	recalculate_rects();
	invalidate();
}

Window::~Window() {
	for(auto& child : _children)
		delete child;
	_display->remove_window(this);
	if(_client)
		_client->window_destroyed(this);
	invalidate();
	if(_framebuffer.data) {
		//Deallocate the old framebuffer since there is one
		if(shmdetach(_framebuffer_shm.id) < 0) {
			perror("Failed to deallocate framebuffer for window");
			return;
		}
	}
}

Window* Window::parent() const {
	return _parent;
}

Client* Window::client() const {
	return _client;
}

void Window::set_client(Client* client) {
	_client = client;
}

int Window::id() const {
	return _id;
}

Image Window::framebuffer() const {
	return _framebuffer;
}

Display* Window::display() const {
	return _display;
}

bool Window::is_decoration() const {
	return false;
}

Rect Window::rect() const {
	return _rect;
}

Rect Window::absolute_rect() const {
	return _absolute_rect;
}

Rect Window::visible_absolute_rect() const {
	return _visible_absolute_rect;
}

void Window::set_dimensions(const Dimensions& dims) {
	invalidate();
	_rect = {_rect.x, _rect.y, dims.width, dims.height};
	alloc_framebuffer();
	recalculate_rects();
	invalidate();
}

void Window::set_position(const Point& position) {
	invalidate();
	_rect = {position.x, position.y, _rect.width, _rect.height};
	recalculate_rects();
	invalidate();
}

void Window::invalidate() {
	_display->invalidate(_visible_absolute_rect);
}

void Window::invalidate(const Rect& area) {
	//TODO: Check if there's an overlap between already invalidated areas
	if(_parent)
		_display->invalidate(area.transform(_absolute_rect.position()).overlapping_area(_parent->_absolute_rect));
	else
		_display->invalidate(area.transform(_absolute_rect.position()).overlapping_area(_display->dimensions()));
}

void Window::move_to_front() {
	_display->move_to_front(this);
	for(auto child : _children)
		child->move_to_front();
}

void Window::focus() {
	_display->focus(this);
}

void Window::mouse_moved(Point relative_pos, int delta_x, int delta_y) {
	_mouse_position = relative_pos;
	if(_client)
		_client->mouse_moved(this, relative_pos);
}

void Window::set_mouse_buttons(uint8_t buttons) {
	_mouse_buttons = buttons;
	if(buttons != _mouse_buttons && _client)
		_client->mouse_buttons_changed(this, buttons);
}

uint8_t Window::mouse_buttons() {
	return _mouse_buttons;
}

void Window::set_global_mouse(bool global) {
	_global_mouse = global;
}

bool Window::gets_global_mouse() {
	return _global_mouse;
}

void Window::handle_keyboard_event(const KeyboardEvent& event) {
	if(_client)
		_client->keyboard_event(this, event);
	else if(_parent)
		_parent->handle_keyboard_event(event);
}

void Window::alloc_framebuffer() {
	if(_framebuffer.data) {
		//Deallocate the old framebuffer since there is one
		if(shmdetach(_framebuffer_shm.id) < 0) {
			perror("Failed to deallocate framebuffer for window");
			return;
		}
	}

	if(shmcreate(NULL, IMGSIZE(_rect.width, _rect.height), &_framebuffer_shm) < 0) {
		perror("Failed to allocate framebuffer for window");
		return;
	}

	_framebuffer = {(uint32_t*) _framebuffer_shm.ptr, _rect.width, _rect.height};
}

Rect Window::calculate_absolute_rect(const Rect& rect) {
	if(_parent)
		return _parent->calculate_absolute_rect(rect.transform(_parent->_rect.position()));
	else
		return rect;
}

void Window::recalculate_rects() {
	_absolute_rect = calculate_absolute_rect(_rect);
	if(_parent)
		_visible_absolute_rect = _absolute_rect.overlapping_area(_parent->_visible_absolute_rect);
	else
		_visible_absolute_rect = _absolute_rect.overlapping_area(_display->dimensions());
	for(auto child : _children)
		child->recalculate_rects();
}

shm Window::framebuffer_shm() {
	return _framebuffer_shm;
}

const char* Window::title() {
	return _title ? _title : "";
}

void Window::set_title(const char* title) {
	delete _title;
	_title = strdup(title);
	if(_parent && _parent->is_decoration())
		((DecorationWindow*) _parent)->redraw_frame();
}


