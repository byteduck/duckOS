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

int Window::destroy() {
	if(!context->send_packet(PDestroyWindowPkt(id))) {
		perror("Pond: Failed to write destroy window packet");
		return -1;
	}
	return 0;
}

void Window::invalidate() {
	if(!context->send_packet(PInvalidatePkt(id, -1, -1, -1, -1)))
		perror("Pond: Failed to write invalidate packet");
}

void Window::invalidate_area(int area_x, int area_y, int area_width, int area_height) {
	if(!context->send_packet(PInvalidatePkt(id, area_x, area_y, area_width, area_height)))
		perror("Pond: Failed to write invalidate area packet");
}

void Window::resize(int width, int height) {
	if(!context->send_packet(PResizeWindowPkt(id, width, height)))
		return perror("Pond: failed to write resize window packet");
	//TODO This is hacky and will break things.
	context->next_event();
	this->width = width;
	this->height = height;
}

void Window::set_position(int x, int y) {
	if(!context->send_packet(PMoveWindowPkt(id, x, y)))
		return perror("Pond: failed to write resize window packet");
	//TODO This is hacky and will break things.
	context->next_event();
	this->x = x;
	this->x = y;
}

void Window::set_title(const char* title) {
	if(!context->send_packet(PSetTitlePkt(id, title)))
		perror("Pond: failed to write set title packet");
}

void Window::reparent(Window* window) {
	if(!context->send_packet(PReparentPkt(id, window ? window->id : 0)))
		perror("Pond: failed to write set parent packet");
}
