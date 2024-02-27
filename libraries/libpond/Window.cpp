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
#include "Context.h"
#include <libgraphics/Image.h>

using namespace Pond;
using namespace Gfx;

Window::Window(int id, Gfx::Rect rect, struct shm shm, Context* ctx): _id(id), _rect(rect), _context(ctx), _shm(shm) {}

Window::~Window() = default;

void Window::destroy() {
	_context->__river_destroy_window({_id});
}

void Window::invalidate() {
	_flipped = _context->__river_invalidate_window({_id, {-1, -1, -1, -1}});
}

void Window::invalidate_area(Gfx::Rect area) {
	_flipped = _context->__river_invalidate_window({_id, area});
}

void Window::resize(Gfx::Dimensions dims) {
	auto resp = _context->__river_resize_window({_id, dims});
	Event evt;
	_context->handle_window_resized(resp, evt);
}

void Window::resize(int width, int height) {
	resize({width, height});
}

void Window::set_position(Gfx::Point pos) {
	_context->__river_move_window({_id, pos});
	_rect.x = pos.x;
	_rect.y = pos.y;
}

void Window::set_position(int x, int y) {
	set_position({x, y});
}

Gfx::Point Window::position() const {
	return _rect.position();
}

Gfx::Dimensions Window::dimensions() const {
	return _rect.dimensions();
}

Gfx::Rect Window::rect() const {
	return _rect;
}

void Window::set_title(const char* title) {
	_context->__river_set_title({_id, title});
}

void Window::reparent(Window* window) {
	_context->__river_reparent({_id, window ? window->_id : 0});
}

void Window::set_global_mouse(bool global) {
	_context->__river_set_hint({_id, PWINDOW_HINT_GLOBALMOUSE, global});
}

void Window::set_draggable(bool draggable) {
	_context->__river_set_hint({_id, PWINDOW_HINT_DRAGGABLE, draggable});
}

void Window::set_resizable(bool resizable) {
	_context->__river_set_hint({_id, PWINDOW_HINT_RESIZABLE, resizable});
}

void Window::bring_to_front() {
	_context->__river_window_to_front({_id});
}

void Window::set_hidden(bool hidden) {
	_context->__river_set_hint({_id, PWINDOW_HINT_HIDDEN, hidden});
	_hidden = hidden;
}

void Window::set_uses_alpha(bool alpha_blending) {
	_context->__river_set_hint({_id, PWINDOW_HINT_USEALPHA, alpha_blending});
}

void Window::set_uses_alpha_hit_testing(bool alpha_hit_testing) {
	_context->__river_set_hint({_id, PWINDOW_HINT_ALPHA_HITTEST, alpha_hit_testing});
}

int Window::id() const {
	return _id;
}

Framebuffer Window::framebuffer() const {
	return {(Gfx::Color*) _shm.ptr + (_flipped ? 0 : _rect.width * _rect.height), _rect.width, _rect.height};
}

unsigned int Window::mouse_buttons() const {
	return _mouse_buttons;
}

Gfx::Point Window::mouse_pos() const {
	return _mouse_pos;
}

WindowType Window::type() const {
	return _window_type;
}

void Window::set_type(WindowType type) {
	_context->__river_set_hint({_id, PWINDOW_HINT_WINDOWTYPE, (int) type});
	_window_type = type;
}

void Window::focus() {
	_context->__river_focus_window({_id});
}

void Window::set_has_shadow(bool shadow) {
	_context->__river_set_hint({_id, PWINDOW_HINT_SHADOW, (int) shadow});
}

void Window::set_minimum_size(Gfx::Dimensions dimensions) {
	_context->__river_set_minimum_size({_id, dimensions});
}
