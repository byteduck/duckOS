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
#include <sys/mem.h>
#include <cstdio>

Window::Window(Window* parent, const Rect& rect): _parent(parent), _rect(rect), _display(parent->_display) {
	alloc_framebuffer();
	_parent->_children.push_back(this);
	_display->add_window(this);
}

Window::Window(Display* display): _parent(nullptr), _rect(display->dimensions()), _display(display) {
	alloc_framebuffer();
	_display->add_window(this);
}

Window::~Window() {
	_display->remove_window(this);
}

Window* Window::parent() const {
	return _parent;
}

Framebuffer Window::framebuffer() const {
	return _framebuffer;
}

Rect Window::rect() const {
	return _rect;
}

Rect Window::absolute_rect() const {
	return _absolute_rect;
}

void Window::set_rect(const Rect& rect) {
	invalidate();
	if(_parent)
		_rect = rect.constrain_relative(_parent->_rect);
	else
		_rect = rect;
	_absolute_rect = calculate_absolute_rect();
	invalidate();
}

void Window::set_position(const Point& position) {
	invalidate();
	Rect new_dims = {position.x, position.y, _rect.height, _rect.width};
	if(_parent)
		_rect = new_dims.constrain_relative(_parent->_rect);
	else
		_rect = new_dims;
	_absolute_rect = calculate_absolute_rect();
	invalidate();
}

void Window::invalidate() {
	invalidate({0, 0, _rect.width, _rect.height});
}

void Window::invalidate(const Rect& area) {
	//TODO: Check if there's an overlap between already invalidated areas
	_display->invalidate(area.transform(_absolute_rect.position));
}

void Window::alloc_framebuffer() {
	shm shminfo;

	if(shmcreate(NULL, _rect.width * _rect.height * sizeof(Color), &shminfo) < 0) {
		perror("Failed to allocate framebuffer for window");
		return;
	}

	_framebuffer = {(Color*) shminfo.ptr, _rect.width, _rect.height};
}

Rect Window::calculate_absolute_rect() {
	if(_parent)
		return _rect.transform(_parent->_rect.position);
	else
		return _rect;
}
