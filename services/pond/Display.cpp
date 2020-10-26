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

#include "Display.h"
#include <unistd.h>
#include <cstdio>
#include <sys/ioctl.h>
#include <kernel/device/VGADevice.h>
#include <cstring>
#include <sys/time.h>

Display* Display::_inst = nullptr;

Display::Display(): _dimensions({0, 0, 0, 0}) {
	_inst = this;
	framebuffer_fd = open("/dev/fb0", O_RDWR);
	if(framebuffer_fd < -1) {
		perror("Failed to open framebuffer");
		return;
	}

	if(ioctl(framebuffer_fd, IO_VIDEO_WIDTH, &_dimensions.width) < 0) {
		perror("Failed to get framebuffer width");
		return;
	}

	if(ioctl(framebuffer_fd, IO_VIDEO_HEIGHT, &_dimensions.height) < 0) {
		perror("Failed to get framebuffer height");
		return;
	}

	Color* buffer;

	if(ioctl(framebuffer_fd, IO_VIDEO_MAP, &buffer) < 0) {
		perror("Failed to map framebuffer");
		return;
	}

	_framebuffer = {buffer, _dimensions.width, _dimensions.height};

	printf("Framebuffer opened and mapped (%d x %d).\n", _dimensions.width, _dimensions.height);
}

Rect Display::dimensions() {
	return _dimensions;
}

Framebuffer Display::framebuffer() {
	return _framebuffer;
}

void Display::clear(Color color) {
	size_t framebuffer_size = _dimensions.area();
	for(size_t i = 0; i < framebuffer_size; i++) {
		_framebuffer.buffer[i] = color;
	}
}

void Display::set_root_window(Window* window) {
	_root_window = window;
}

Window* Display::root_window() {
	return _root_window;
}

void Display::set_mouse_window(Mouse* window) {
	_mouse_window = window;
}

Mouse* Display::mouse_window() {
	return _mouse_window;
}

void Display::add_window(Window* window) {
	_windows.push_back(window);
}

void Display::remove_window(Window* window) {
	for(size_t i = 0; i < _windows.size(); i++) {
		if(_windows[i] == window) {
			_windows.erase(_windows.begin() + i);
			return;
		}
	}
}

void Display::invalidate(const Rect& rect) {
	if(!rect.empty())
		invalid_areas.push_back(rect);
}

timeval paint_time = {0, 0};
bool display_buffer_dirty = false;

void Display::repaint() {
	auto fb = _root_window->framebuffer();

	if(!invalid_areas.empty())
		display_buffer_dirty = true;
	else
		return;

	for(auto& area : invalid_areas) {
		// Fill the invalid area with the background color.
		fb.fill(area, {50, 50, 50});

		// See if each window overlaps the invalid area.
		for(auto window : _windows) {
			//Don't bother with the mouse window, we draw it separately so it's always on top
			if(window == _mouse_window)
				continue;

			Rect window_vabs = window->visible_absolute_rect();
			if(window_vabs.collides(area)) {
				//If it does, redraw the intersection of the window in question and the invalid area
				Rect window_abs = window->absolute_rect();
				Rect overlap_abs = area.overlapping_area(window_vabs);
				fb.copy(window->framebuffer(), overlap_abs.transform({-window_abs.x, -window_abs.y}), overlap_abs.position());
			}
		}
	}
	invalid_areas.resize(0);

	//Draw the mouse.
	fb.copy(_mouse_window->framebuffer(), {0, 0, _mouse_window->rect().width, _mouse_window->rect().height}, _mouse_window->absolute_rect().position());
}

void Display::flip_buffers() {
	//If the screen buffer isn't dirty, don't bother
	if(!display_buffer_dirty)
		return;

	//If it hasn't been 1/60 of a second since the last buffer flip, don't copy to the display buffer
	timeval new_time = {0, 0};
	gettimeofday(&new_time, NULL);
	if(new_time.tv_sec == paint_time.tv_sec && new_time.tv_usec - paint_time.tv_usec < 16666)
		return;
	paint_time = new_time;

	//Copy to the graphics buffer
	memcpy(_framebuffer.buffer, _root_window->framebuffer().buffer, sizeof(Color) * _framebuffer.width * _framebuffer.height);
}

void Display::move_to_front(Window* window) {
	for (auto it = _windows.begin(); it != _windows.end(); it++) {
		if(*it == window) {
			_windows.erase(it);
			_windows.push_back(window);
			window->invalidate();
			return;
		}
	}
}

void Display::create_mouse_events(int delta_x, int delta_y, uint8_t buttons) {
	Point mouse = _mouse_window->absolute_rect().position();
	for (auto it = _windows.rbegin(); it != _windows.rend(); it++) {
		auto* window = *it;
		if(window == _mouse_window || window == _root_window)
			continue;
		if(mouse.in(window->absolute_rect())) {
			Point window_pos = window->absolute_rect().position();
			window->mouse_moved({mouse.x - window_pos.x, mouse.y - window_pos.y}, delta_x, delta_y);
			window->set_mouse_buttons(_mouse_window->mouse_buttons());
			return;
		}
	}
}

Display& Display::inst() {
	return *_inst;
}
