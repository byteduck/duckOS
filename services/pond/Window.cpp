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
#include "Display.h"

Window::Window(Window* parent, const Rect& rect): _parent(parent), _rect(rect), _display(parent->_display) {
	alloc_framebuffer();
	_parent->_children.push_back(this);
	_display->add_window(this);
	recalculate_rects();
	invalidate();
}

Window::Window(Display* display): _parent(nullptr), _rect(display->dimensions()), _display(display) {
	alloc_framebuffer();
	_display->set_root_window(this);
	recalculate_rects();
	invalidate();
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

Display* Window::display() const {
	return _display;
}

bool Window::is_decorated() const {
	return _decorated;
}

void Window::set_decorated(bool decorated) {
	_decorated = decorated;
	invalidate();
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

void Window::alloc_framebuffer() {
	shm shminfo;

	if(shmcreate(NULL, _rect.width * _rect.height * sizeof(Color), &shminfo) < 0) {
		perror("Failed to allocate framebuffer for window");
		return;
	}

	_framebuffer = {(Color*) shminfo.ptr, _rect.width, _rect.height};
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


