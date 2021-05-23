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
#include "Context.h"
#include <cstdio>

using namespace Pond;

Window::Window(int id, Rect rect, struct shm shm, Context* ctx): _id(id), _rect(rect), _context(ctx), _shm_id(shm.id) {
	_framebuffer = {(uint32_t*) shm.ptr, rect.width, rect.height};
}

int Window::destroy() {
	if(!_context->send_packet(PDestroyWindowPkt(_id))) {
		perror("Pond: Failed to write destroy window packet");
		return -1;
	}
	return 0;
}

void Window::invalidate() {
	if(!_context->send_packet(PInvalidatePkt(_id, {-1, -1, -1, -1})))
		perror("Pond: Failed to write invalidate packet");
}

void Window::invalidate_area(Rect area) {
	if(!_context->send_packet(PInvalidatePkt(_id, area)))
		perror("Pond: Failed to write invalidate area packet");
}

void Window::resize(Dimensions dims) {
	if(!_context->send_packet(PResizeWindowPkt(_id, dims)))
		return perror("Pond: failed to write set_rect window packet");
	_context->next_event(PEVENT_WINDOW_RESIZE);
	_rect.width = dims.width;
	_rect.height = dims.height;
}

void Window::resize(int width, int height) {
	resize({width, height});
}

void Window::set_position(Point pos) {
	if(!_context->send_packet(PMoveWindowPkt(_id, pos)))
		return perror("Pond: failed to write set_rect window packet");
	_context->next_event(PEVENT_WINDOW_MOVE);
	_rect.x = pos.x;
	_rect.y = pos.y;
}

void Window::set_position(int x, int y) {
	set_position({x, y});
}

Point Window::position() const {
	return _rect.position();
}

Dimensions Window::dimensions() const {
	return _rect.dimensions();
}

Rect Window::rect() const {
	return _rect;
}

void Window::set_title(const char* title) {
	if(!_context->send_packet(PSetTitlePkt(_id, title)))
		perror("Pond: failed to write set title packet");
}

void Window::reparent(Window* window) {
	if(!_context->send_packet(PReparentPkt(_id, window ? window->_id : 0)))
		perror("Pond: failed to write set parent packet");
}

void Window::set_global_mouse(bool global) {
	if(!_context->send_packet(PWindowHintPkt(_id, PWINDOW_HINT_GLOBALMOUSE, global)))
		perror("Pond: failed to write set hint packet");
}

void Window::set_draggable(bool draggable) {
	if(!_context->send_packet(PWindowHintPkt(_id, PWINDOW_HINT_DRAGGABLE, draggable)))
		perror("Pond: failed to write set hint packet");
}

void Window::set_resizable(bool resizable) {
    if(!_context->send_packet(PWindowHintPkt(_id, PWINDOW_HINT_RESIZABLE, resizable)))
        perror("Pond: failed to write set hint packet");
}

void Window::bring_to_front() {
	if(!_context->send_packet(PWindowToFrontPkt(_id)))
		perror("Pond: failed to write bring to front packet");
}

void Window::set_hidden(bool hidden) {
	if(!_context->send_packet(PWindowHintPkt(_id, PWINDOW_HINT_HIDDEN, hidden)))
		perror("Pond: failed to write window hint packet");
	_hidden = hidden;
}

void Window::set_uses_alpha(bool alpha_blending) {
	if(!_context->send_packet(PWindowHintPkt(_id, PWINDOW_HINT_USEALPHA, alpha_blending)))
		perror("Pond: failed to write window hint packet");
}

int Window::id() const {
	return _id;
}

Image Window::framebuffer() const {
	return _framebuffer;
}

unsigned int Window::mouse_buttons() const {
	return _mouse_buttons;
}

Point Window::mouse_pos() const {
	return _mouse_pos;
}
