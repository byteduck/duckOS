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

#include "Window.h"
#include <cstdio>
#include "Display.h"
#include <libgraphics/Image.h>
#include <libduck/Log.h>
#include <memory.h>
#include <libpond/Window.h>

using namespace Gfx;

int Window::current_id = 0;

#define SHADOW_SIZE 6
static int SHADOW_ALPH = 400 / (double) (SHADOW_SIZE * SHADOW_SIZE * 4);

Window::Window(Window* parent, const Gfx::Rect& rect, bool hidden): _parent(parent), _rect(rect), _display(parent->_display), _id(++current_id), _hidden(hidden) {
	if(_rect.width < 1)
		_rect.width = 1;
	if(_rect.height < 1)
		_rect.height = 1;
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
	_destructing = true;
	for(auto& child : _children)
		delete child;
	_display->remove_window(this);
	if(_client)
		_client->window_destroyed(this);
	if(_parent)
		_parent->remove_child(this);
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

void Window::reparent(Window* new_parent) {
	//If we have a parent, remove ourself from its children
	if(_parent) {
		for(auto it = _parent->_children.begin(); it < _parent->_children.end(); it++) {
			if(*it == this)
				_parent->_children.erase(it);
		}
		_parent->invalidate(_rect);
	}

	//If new_parent isn't null, add ourselves to the new parent's children
	if(new_parent) {
		new_parent->_children.push_back(this);
	}

	//Set the parent and recalculate/invalidate
	_parent = new_parent;
	recalculate_rects();
	invalidate();
}

void Window::remove_child(Window* child) {
	//No sense in wasting time removing children if we're destroying this window
	if(_destructing)
		return;
	auto child_it = std::find(_children.begin(), _children.end(), child);
	if(child_it != _children.end())
		_children.erase(child_it);
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

const Framebuffer& Window::framebuffer() const {
	return _framebuffer;
}

Display* Window::display() const {
	return _display;
}

Gfx::Rect Window::rect() const {
	return _rect;
}

Gfx::Rect Window::absolute_rect() const {
	return _absolute_rect;
}

Gfx::Rect Window::absolute_shadow_rect() const {
	return _absolute_shadow_rect;
}

void Window::set_dimensions(const Gfx::Dimensions& new_dims, bool notify_client) {
	if(new_dims.width == _rect.dimensions().width && new_dims.height == _rect.dimensions().height)
		return;
	Gfx::Dimensions dims = {
		std::max(_minimum_size.width, new_dims.width),
		std::max(_minimum_size.height, new_dims.height)
	};
	invalidate();
	_rect = {_rect.x, _rect.y, dims.width, dims.height};
	alloc_framebuffer();
	recalculate_rects();
	invalidate();
	if(notify_client && _client)
		_client->window_resized(this);
}

void Window::set_position(const Gfx::Point& position, bool notify_client) {
	invalidate();
	_rect = {position.x, position.y, _rect.width, _rect.height};
	recalculate_rects();
	invalidate();
	if(notify_client && _client)
		_client->window_moved(this);
}

void Window::set_rect(const Gfx::Rect& rect, bool notify_client) {
	invalidate();
	_rect = rect;
	_rect.set_dimensions({
		std::max(_rect.width, _minimum_size.width),
		std::max(_rect.height, _minimum_size.height)
	});
	alloc_framebuffer();
	recalculate_rects();
	invalidate();
	if(notify_client && _client)
		_client->window_resized(this);
}

void Window::invalidate() {
	if(!hidden()) {
		if(_parent)
			_display->invalidate(_absolute_shadow_rect.overlapping_area(_parent->_absolute_rect));
		else
			_display->invalidate(_absolute_shadow_rect.overlapping_area(_display->dimensions()));
	}
}

void Window::invalidate(const Gfx::Rect& area) {
	if(hidden())
		return;
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

void Window::mouse_moved(Gfx::Point delta, Gfx::Point relative_pos, Gfx::Point absolute_pos) {
	_mouse_position = relative_pos;
	if(_client)
		_client->mouse_moved(this, delta, relative_pos, absolute_pos);
}

void Window::set_mouse_buttons(uint8_t buttons) {
	if(buttons != _mouse_buttons && _client) {
		_mouse_buttons = buttons;
		_client->mouse_buttons_changed(this, buttons);
	} else {
		_mouse_buttons = buttons;
	}
}

void Window::mouse_scrolled(int scroll) {
	_client->mouse_scrolled(this, scroll);
}

void Window::mouse_left() {
	if(_client)
		_client->mouse_left(this);
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

void Window::set_draggable(bool draggable) {
	_draggable = draggable;
}

bool Window::draggable() {
	return _draggable;
}

void Window::set_resizable(bool resizable) {
	_resizable = resizable;
}

bool Window::resizable() {
	return _resizable;
}

void Window::set_hidden(bool hidden) {
	if(hidden == _hidden)
		return;
	if(_parent)
		_display->invalidate(_absolute_shadow_rect.overlapping_area(_parent->_absolute_rect));
	else
		_display->invalidate(_absolute_shadow_rect.overlapping_area(_display->dimensions()));
	_hidden = hidden;
	_display->window_hidden(this);
}

bool Window::hidden() {
	if(_hidden)
		return true;
	else if(_parent)
		return _parent->hidden();
	else
		return false;
}

bool Window::uses_alpha() {
	return _uses_alpha;
}

void Window::handle_keyboard_event(const KeyboardEvent& event) {
	if(_client)
		_client->keyboard_event(this, event);
	else if(_parent)
		_parent->handle_keyboard_event(event);
}

Gfx::Rect Window::calculate_absolute_rect(const Gfx::Rect& rect) {
	if(_parent)
		return _parent->calculate_absolute_rect(rect.transform(_parent->_rect.position()));
	else
		return rect;
}

void Window::set_flipped(bool flipped) {
	_framebuffer = {
			(Gfx::Color*) _framebuffer_shm.ptr + (flipped ? _rect.width * _rect.height : 0),
			_rect.width,
			_rect.height
	};
}

void Window::alloc_framebuffer() {
	if(_framebuffer.data) {
		//Deallocate the old framebuffer since there is one
		if(shmdetach(_framebuffer_shm.id) < 0) {
			perror("Failed to deallocate framebuffer for window");
			return;
		}
	}

	if(shmcreate(NULL, IMGSIZE(_rect.width, _rect.height) * 2, &_framebuffer_shm) < 0) {
		perror("Failed to allocate framebuffer for window");
		return;
	}

	_framebuffer = {(Gfx::Color*) _framebuffer_shm.ptr, _rect.width, _rect.height};

	// Now, allocate and draw the shadow buffer
	_shadow_buffer = Gfx::Framebuffer(_rect.width + SHADOW_SIZE * 2, _rect.height + SHADOW_SIZE * 2);

	// Poor man's box-shadow :)
	Rect shadow_rect = {0, 0, _shadow_buffer.width, _shadow_buffer.height};
	_shadow_buffer.fill(shadow_rect, RGBA(0,0,0,0));
	Rect window_rect = shadow_rect.inset(SHADOW_SIZE);
	auto box_blur_rect = [&](Gfx::Rect rect) {
		for(int y = rect.y; y < rect.height + rect.y; y++) {
			for(int x = rect.x; x < rect.width + rect.x; x++) {
				auto sample_rect = Rect {x - SHADOW_SIZE + 1, y - SHADOW_SIZE + 1, SHADOW_SIZE * 2 - 2, SHADOW_SIZE * 2 - 2}.overlapping_area(shadow_rect);
				int alph = 0;
				for(int sample_y = sample_rect.y; sample_y < sample_rect.height + sample_rect.y; sample_y++)
					for(int sample_x = sample_rect.x; sample_x < sample_rect.width + sample_rect.x; sample_x++)
						if(Point {sample_x, sample_y}.in(window_rect))
							alph += SHADOW_ALPH;
				*_shadow_buffer.at({x, y}) = RGBA(0,0,0,alph);
			}
		}
	};

	box_blur_rect({SHADOW_SIZE, 0, _shadow_buffer.width - SHADOW_SIZE * 2, SHADOW_SIZE});
	box_blur_rect({SHADOW_SIZE, _shadow_buffer.height - SHADOW_SIZE, _shadow_buffer.width - SHADOW_SIZE * 2, SHADOW_SIZE});
	box_blur_rect({0, 0, SHADOW_SIZE, _shadow_buffer.height});
	box_blur_rect({_shadow_buffer.width - SHADOW_SIZE, 0, SHADOW_SIZE, _shadow_buffer.height});
}

void Window::recalculate_rects() {
	_absolute_rect = calculate_absolute_rect(_rect);
	_absolute_shadow_rect = _absolute_rect.inset(_draws_shadow ? -SHADOW_SIZE : 0);
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
}

void Window::set_hint(int hint, int value) {
	switch(hint) {
		case PWINDOW_HINT_GLOBALMOUSE:
			set_global_mouse(value);
			break;
		case PWINDOW_HINT_DRAGGABLE:
			set_draggable(value);
			break;
		case PWINDOW_HINT_HIDDEN:
			set_hidden(value);
			break;
		case PWINDOW_HINT_USEALPHA:
			if(_uses_alpha != (bool) value) {
				_uses_alpha = value;
				invalidate();
			}
			break;
		case PWINDOW_HINT_RESIZABLE:
			set_resizable(value);
			break;
		case PWINDOW_HINT_WINDOWTYPE:
			if(value >= Pond::WindowType::DEFAULT && value <= Pond::WindowType::MENU)
				set_type((Pond::WindowType) value);
			break;
		case PWINDOW_HINT_SHADOW:
			set_has_shadow(value);
			break;
		default:
			Duck::Log::warn("Unknown window hint ", hint);
	}
}

Pond::WindowType Window::type() {
	return _type;
}

void Window::set_type(Pond::WindowType type) {
	_type = type;
}

void Window::notify_focus(bool focus) {
	if(_client)
		_client->window_focused(this, focus);
}

bool Window::has_shadow() const {
	return _draws_shadow;
}

void Window::set_has_shadow(bool shadow) {
	_draws_shadow = shadow;
	recalculate_rects();
	invalidate();
}

void Window::set_minimum_size(Gfx::Dimensions minimum) {
	_minimum_size = {
		std::max(minimum.width, WINDOW_RESIZE_BORDER * 2),
		std::max(minimum.height, WINDOW_RESIZE_BORDER * 2)
	};
}

