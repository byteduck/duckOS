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
	Copyright (c) ChazizGRKB 2022.
*/

#include "Display.h"
#include "FontManager.h"
#include <libgraphics/Image.h>
#include <libgraphics/PNG.h>
#include <libduck/Log.h>
#include <libduck/Config.h>
#include <unistd.h>
#include <cstdio>
#include <sys/ioctl.h>
#include <kernel/device/VGADevice.h>
#include <sys/input.h>
#include <libgraphics/Memory.h>

using namespace Gfx;
using Duck::Log, Duck::Config, Duck::ResultRet;

Display* Display::_inst = nullptr;

Display::Display(): _dimensions({0, 0, 0, 0}) {
	_inst = this;
	framebuffer_fd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
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

	Gfx::Color* buffer;

	if(ioctl(framebuffer_fd, IO_VIDEO_MAP, &buffer) < 0) {
		perror("Failed to map framebuffer");
		return;
	}

	//If we're running in a tty, set it to graphical mode
	ioctl(STDOUT_FILENO, TIOSGFX, nullptr);

	//If we can set the offset into video memory, that means we can flip the display buffer and write directly to it
	if(!ioctl(framebuffer_fd, IO_VIDEO_OFFSET, 0))
		_buffer_mode = BufferMode::DoubleFlip;
	else
		_buffer_mode = BufferMode::Double;

	_framebuffer = {buffer, _dimensions.width, _dimensions.height};
	Log::info("Display opened and mapped (", _dimensions.width, " x ", _dimensions.height, ")");

	if((_keyboard_fd = open("/dev/input/keyboard", O_RDONLY | O_CLOEXEC)) < 0)
		perror("Failed to open keyboard");

	gettimeofday(&paint_time, NULL);
}

Gfx::Rect Display::dimensions() {
	return _dimensions;
}

Framebuffer& Display::framebuffer() {
	return _framebuffer;
}

void Display::clear(uint32_t color) {
	size_t framebuffer_size = _dimensions.area();
	for(size_t i = 0; i < framebuffer_size; i++) {
		_framebuffer.data[i] = color;
	}
}

Duck::Result Display::load_config() {
	auto cfg = TRY(Duck::Config::read_from("/etc/pond.conf"));
	if(cfg.has_section("desktop")) {
		auto desktop = cfg.section("desktop");
		if(!desktop["background"].empty()) {
			int num = sscanf(desktop["background"].c_str(), "#%lx , #%lx", &_background_a, &_background_b);
			if(!num) {
				auto wallpaper_res = Gfx::Image::load(desktop["background"]);
				if(wallpaper_res.has_value())
					_wallpaper = wallpaper_res.value();
			} else if(num == 1) {
				_background_b = _background_a;
			}
		}
	}
	return Duck::Result::SUCCESS;
}

void Display::set_root_window(Window* window) {
	_root_window = window;
	load_config();
	_root_window->invalidate();
	_background_framebuffer = Framebuffer(_root_window->rect().width, _root_window->rect().height);
	auto& fb = _background_framebuffer;
	if(_wallpaper)
		_wallpaper->draw(fb, {0, 0, fb.width, fb.height});
	else
		fb.fill_gradient_v({0, 0, fb.width, fb.height}, _background_a, _background_b);
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
	if(window == _focused_window)
		_focused_window = nullptr;
	if(window == _prev_mouse_window)
		_prev_mouse_window = nullptr;
	if(window == _drag_window)
		_drag_window = nullptr;
	if(window == _resize_window)
		_resize_window = nullptr;
	if(window == _mousedown_window)
		_mousedown_window = nullptr;
	for(size_t i = 0; i < _windows.size(); i++) {
		if(_windows[i] == window) {
			_windows.erase(_windows.begin() + i);
			return;
		}
	}
}

void Display::invalidate(const Gfx::Rect& rect) {
	if(!rect.empty())
		invalid_areas.push_back(rect);
}

//#define DEBUG_REPAINT_PERF
void Display::repaint() {
#ifdef DEBUG_REPAINT_PERF
	timeval t0, t1;
	gettimeofday(&t0, nullptr);
#endif

	if(!invalid_areas.empty())
		display_buffer_dirty = true;
	else
		return;

	//If it hasn't been 1/60 of a second since the last repaint, don't bother
	if(millis_until_next_flip())
		return;
	gettimeofday(&paint_time, NULL);

	auto& fb = _buffer_mode == BufferMode::Single ? _framebuffer : _root_window->framebuffer();

	//Combine areas that overlap
	auto it = invalid_areas.begin();
	while(it != invalid_areas.end()) {
		bool remove_area = false;
		for(auto & other_area : invalid_areas) {
			if(&*it != &other_area && it->collides(other_area)) {
				other_area = it->combine(other_area);
				remove_area = true;
				break;
			}
		}
		if(remove_area)
			invalid_areas.erase(it);
		else
			it++;
	}

	//If double buffering, combine the invalid areas together to calculate the portion of the framebuffer to be redrawn
	if(_buffer_mode == BufferMode::Double) {
		//If the invalid buffer area is empty (has an x of -1), initialize it to the first invalid area
		if(_invalid_buffer_area.x == -1)
			_invalid_buffer_area = invalid_areas[0];
		for(auto& area : invalid_areas)
			_invalid_buffer_area = _invalid_buffer_area.combine(area);
	}

	for(auto& area : invalid_areas) {
		// Fill the invalid area with the background.
		fb.copy(_background_framebuffer, area, area.position());

		// See if each window overlaps the invalid area.
		for(auto window : _windows) {
			//Don't bother with the mouse window or hidden windows, we draw it separately so it's always on top
			if(window == _mouse_window || window->hidden())
				continue;

			auto window_shabs = window->absolute_shadow_rect();
			if(window_shabs.collides(area)) {
				//If it does, redraw the intersection of the window in question and the invalid area
				Gfx::Rect window_abs = window->absolute_rect();
				Gfx::Rect overlap_abs = area.overlapping_area(window_abs);
				auto transformed_overlap = overlap_abs.transform({-window_abs.x, -window_abs.y});
				if(window->uses_alpha())
					fb.copy_blitting(window->framebuffer(), transformed_overlap, overlap_abs.position());
				else
					fb.copy(window->framebuffer(), transformed_overlap, overlap_abs.position());

				//Draw the shadow
				if(window->has_shadow()) {
					auto draw_partial_shadow = [&](Rect rect) {
						Gfx::Rect shadow_abs = area.overlapping_area(rect);
						auto transformed_shadow = shadow_abs.transform({-window_shabs.x, -window_shabs.y});
						fb.copy_blitting(window->shadow_buffer(), transformed_shadow, shadow_abs.position());
					};
					// Only draw the bits of the shadow buffer outside of the window, no need to fill in a bunch of empty pixels in the middle :)
					auto shadow_size = window_abs.x - window_shabs.x;
					draw_partial_shadow(window_shabs.inset(0, shadow_size, window_shabs.height - shadow_size, shadow_size));
					draw_partial_shadow(window_shabs.inset(window_shabs.height - shadow_size, shadow_size, 0, shadow_size));
					draw_partial_shadow(window_shabs.inset(0, 0, 0, window_shabs.width - shadow_size));
					draw_partial_shadow(window_shabs.inset(0, window_shabs.width - shadow_size, 0, 0));
				}
			}
		}
	}
	invalid_areas.resize(0);

	//If we're resizing a window, draw the outline
	if(_resize_window)
		fb.outline(_resize_rect, RGB(255, 255, 255));

	//Draw the mouse.
	fb.draw_image(_mouse_window->framebuffer(), {0, 0, _mouse_window->rect().width, _mouse_window->rect().height},
				  _mouse_window->absolute_rect().position());

#ifdef DEBUG_REPAINT_PERF
	gettimeofday(&t1, nullptr);
	char buf[10];
	t1.tv_sec -= t0.tv_sec;
	t1.tv_usec -= t0.tv_usec;
	if(t1.tv_usec < 0) {
		t1.tv_sec -= 1 + t1.tv_usec / -1000000;
		t1.tv_usec = (1000000 - (-t1.tv_usec % 1000000)) % 1000000;
	}
	snprintf(buf, 10, "%dms", (int)(t1.tv_usec / 1000 + t1.tv_sec * 1000));
	fb.fill({0, 0, 50, 14}, RGB(0, 0, 0));
	fb.draw_text(buf, {0, 0}, FontManager::inst().get_font("gohu-14"), RGB(255, 255, 255));
#endif

	//Flip the display buffers.
	flip_buffers();
}

bool flipped = false;
void Display::flip_buffers() {
	//If the screen buffer isn't dirty, don't bother
	if(!display_buffer_dirty)
		return;

	if(_buffer_mode == BufferMode::DoubleFlip) {
		auto* video_buf = &_framebuffer.data[flipped ? _framebuffer.height * _framebuffer.width : 0];
		memcpy_uint32((uint32_t*) video_buf, (uint32_t*) _root_window->framebuffer().data, _framebuffer.width * _framebuffer.height);
		ioctl(framebuffer_fd, IO_VIDEO_OFFSET, flipped ? _framebuffer.height : 0);
		flipped = !flipped;
	} else if(_buffer_mode == BufferMode::Double) {
		_framebuffer.copy(_root_window->framebuffer(), _invalid_buffer_area, _invalid_buffer_area.position());
		_invalid_buffer_area.x = -1;
	}

	display_buffer_dirty = false;
}

int Display::millis_until_next_flip() const {
	timeval new_time = {0, 0};
	gettimeofday(&new_time, NULL);
	int diff = (int) (((new_time.tv_sec - paint_time.tv_sec) * 1000000) + (new_time.tv_usec - paint_time.tv_usec))/1000;
	return diff >= 16 ? 0 : 16 - diff;
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

void Display::focus(Window* window) {
	if(window == _focused_window)
		return;
	auto old_focused = _focused_window;
	_focused_window = window;
	if(_focused_window)
		_focused_window->notify_focus(true);
	if(old_focused)
		old_focused->notify_focus(false);
}


void Display::create_mouse_events(int delta_x, int delta_y, int scroll, uint8_t buttons) {
	static uint8_t prev_mouse_buttons = 0;

	Gfx::Point mouse = _mouse_window->absolute_rect().position();
	Gfx::Point delta = {delta_x, delta_y};

	//Drag or stop dragging the current draggable window, if any
	if(_drag_window) {
		if(!(buttons & 1) || !_drag_window->draggable())
			_drag_window = nullptr;
		else
			_drag_window->set_position(_drag_window->rect().position() + delta);
	}

	//If we're resizing the window, check if the user let go of the mouse button or if the window is no longer resizable
	if(_resize_window) {
		//If the mouse has moved at all, we need to change the resize_rect
		if(!(delta_x == 0 && delta_y == 0)) {
			invalidate(_resize_rect);
			_resize_rect = _resize_window->calculate_absolute_rect(calculate_resize_rect());
		}
		if(!_resize_window->resizable()) {
			_resize_window = nullptr;
		} else if(!(buttons & 1)) {
			_resize_window->set_rect(_resize_rect, _resize_mode);
			_resize_window = nullptr;
		}
	}

	//Process global mouse events
	for(auto& window : _windows) {
		if(window->gets_global_mouse()) {
			window->mouse_moved(delta, mouse - window->absolute_rect().position(), mouse);
			if(buttons != prev_mouse_buttons)
				window->set_mouse_buttons(buttons);
			if(scroll)
				window->mouse_scrolled(scroll);
		}
	}

	//If we're moving or resizing a window, don't do anything else
	if(_resize_window || _drag_window)
		return;

	//If we have a mousedown window and released the mouse button, stop sending events to it
	if(_mousedown_window && !(buttons & 1))
		_mousedown_window = nullptr;

	//If we are holding the mouse down, keep sending mouse events to the window we initially clicked
	if(_mousedown_window && !_mousedown_window->gets_global_mouse()) {
		_mousedown_window->mouse_moved(delta, mouse - _mousedown_window->absolute_rect().position(), mouse);
		if(prev_mouse_buttons != buttons)
			_mousedown_window->set_mouse_buttons(buttons);
		if(scroll)
			_mousedown_window->mouse_scrolled(scroll);
		return;
	}

	Window* event_window = nullptr;
	for (auto it = _windows.rbegin(); it != _windows.rend(); it++) {
		auto* window = *it;
		if(window == _mouse_window || window == _root_window || window->hidden())
			continue;

		//If it's near the border, see if we can resize it
		static bool was_near_border = false;
		if(!_resize_window && window->resizable() && mouse.near_border(window->absolute_rect(), WINDOW_RESIZE_BORDER)) {
			was_near_border = true;
			_resize_mode = get_resize_mode(window->absolute_rect(), mouse);
			switch(_resize_mode) {
				case NORTH:
				case SOUTH:
					_mouse_window->set_cursor(Pond::RESIZE_V);
					break;
				case EAST:
				case WEST:
					_mouse_window->set_cursor(Pond::RESIZE_H);
					break;
				case NORTHWEST:
				case SOUTHEAST:
					_mouse_window->set_cursor(Pond::RESIZE_DR);
					break;
				case NORTHEAST:
				case SOUTHWEST:
					_mouse_window->set_cursor(Pond::RESIZE_DL);
					break;
				default:
					_mouse_window->set_cursor(Pond::NORMAL);
			}

			if(!(prev_mouse_buttons & 1) && (buttons & 1) && _resize_mode != NONE) {
				_resize_window = window;
				_resize_begin_point = mouse;
				_resize_rect = window->absolute_rect();
				window->move_to_front();
			}
			break;
		} else if(was_near_border && !_resize_window) {
			was_near_border = false;
			_mouse_window->set_cursor(Pond::NORMAL);
		}

		//Otherwise, if it's in the window, create the appropriate events
		if(mouse.in(window->absolute_rect())) {
			event_window = window;
			if(!window->gets_global_mouse()) {
				window->mouse_moved(delta, mouse - window->absolute_rect().position(), mouse);
				if(prev_mouse_buttons != buttons)
					window->set_mouse_buttons(buttons);
				if(scroll)
					window->mouse_scrolled(scroll);
			}

			//If we mouse down on a window, focus it and set it to the mousedown window
			if(!(prev_mouse_buttons & 1) && (buttons & 1)) {
				window->focus();
				_mousedown_window = window;

				//If the window is draggable, drag it
				if(window->draggable()) {
					_drag_window = window;
					window->move_to_front();
				}
			}
			break;
		}
	}

	// If the mouse was previously in a different window, update the mouse position in that window
	if(event_window != _prev_mouse_window && _prev_mouse_window != nullptr && !_prev_mouse_window->gets_global_mouse()) {
		Gfx::Dimensions window_dims = _prev_mouse_window->rect().dimensions();
		//Constrain the mouse position in the window to the window's rect
		Gfx::Point new_mouse_pos = (mouse - _prev_mouse_window->absolute_rect().position()).constrain({0, 0, window_dims.width, window_dims.height});
		_prev_mouse_window->mouse_moved(delta, new_mouse_pos, new_mouse_pos + _prev_mouse_window->absolute_rect().position());
		_prev_mouse_window->set_mouse_buttons(_mouse_window->mouse_buttons());
		_prev_mouse_window->mouse_left();
	}

	_prev_mouse_window = event_window;
	prev_mouse_buttons = buttons;
}

bool Display::buffer_is_dirty() {
	return display_buffer_dirty;
}

bool Display::update_keyboard() {
	KeyboardEvent events[32];
	ssize_t nread = read(_keyboard_fd, &events, sizeof(KeyboardEvent) * 32);
	if(!nread) return false;
	int num_events = (int) nread / sizeof(KeyboardEvent);
	if(_focused_window) {
		for(int i = 0; i < num_events; i++) {
			_focused_window->handle_keyboard_event(events[i]);
		}
	}
	return true;
}

int Display::keyboard_fd() {
	return _keyboard_fd;
}

void Display::window_hidden(Window* window) {
	if(_mouse_window && _mouse_window->hidden())
		_mouse_window = nullptr;
	if(_focused_window && _focused_window->hidden())
		_focused_window = nullptr;
	if(_drag_window && _drag_window->hidden())
		_drag_window = nullptr;
	if(_resize_window && _resize_window->hidden())
		_resize_window = nullptr;
	if(_mousedown_window && _mousedown_window->hidden())
		_mousedown_window = nullptr;
}

Display& Display::inst() {
	return *_inst;
}

ResizeMode Display::get_resize_mode(Gfx::Rect window, Gfx::Point mouse) {
	if(mouse.in({window.x - WINDOW_RESIZE_BORDER, window.y - WINDOW_RESIZE_BORDER, WINDOW_RESIZE_BORDER * 3, WINDOW_RESIZE_BORDER * 3}))
		return NORTHWEST;
	if(mouse.in({window.x + window.width - WINDOW_RESIZE_BORDER * 2, window.y - WINDOW_RESIZE_BORDER, WINDOW_RESIZE_BORDER * 3, WINDOW_RESIZE_BORDER * 3}))
		return NORTHEAST;
	if(mouse.in({window.x - WINDOW_RESIZE_BORDER, window.y + window.height - WINDOW_RESIZE_BORDER * 2, WINDOW_RESIZE_BORDER * 3, WINDOW_RESIZE_BORDER * 3}))
		return SOUTHWEST;
	if(mouse.in({window.x + window.width - WINDOW_RESIZE_BORDER * 2, window.y + window.height - WINDOW_RESIZE_BORDER * 2, WINDOW_RESIZE_BORDER * 3, WINDOW_RESIZE_BORDER * 3}))
		return SOUTHEAST;
	if(mouse.x < window.x + WINDOW_RESIZE_BORDER)
		return WEST;
	if(mouse.y < window.y + WINDOW_RESIZE_BORDER)
		return NORTH;
	if(mouse.x > window.x + window.width - WINDOW_RESIZE_BORDER)
		return EAST;
	if(mouse.y > window.y + window.height - WINDOW_RESIZE_BORDER)
		return SOUTH;

	return NONE; //Shouldn't happen?
}

Gfx::Rect Display::calculate_resize_rect() {
	if(!_resize_window)
		return {0, 0, 0, 0};

	Gfx::Point new_pos = _resize_window->rect().position();
	Gfx::Dimensions new_dims = _resize_window->rect().dimensions();
	Gfx::Point mouse_delta = _mouse_window->rect().position() - _resize_begin_point;
	switch(_resize_mode) {
		case NORTH:
			new_pos.y += mouse_delta.y;
			new_dims.height -= mouse_delta.y;
			break;
		case SOUTH:
			new_dims.height += mouse_delta.y;
			break;
		case WEST:
			new_pos.x += mouse_delta.x;
			new_dims.width -= mouse_delta.x;
			break;
		case EAST:
			new_dims.width += mouse_delta.x;
			break;
		case NORTHWEST:
			new_pos = new_pos + mouse_delta;
			new_dims.width -= mouse_delta.x;
			new_dims.height -= mouse_delta.y;
			break;
		case NORTHEAST:
			new_pos.y += mouse_delta.y;
			new_dims.width += mouse_delta.x;
			new_dims.height -= mouse_delta.y;
			break;
		case SOUTHWEST:
			new_pos.x += mouse_delta.x;
			new_dims.height += mouse_delta.y;
			new_dims.width -= mouse_delta.x;
			break;
		case SOUTHEAST:
			new_dims.height += mouse_delta.y;
			new_dims.width += mouse_delta.x;
			break;
		case NONE:
			break;
	}
	if(new_dims.height < 1)
		new_dims.height = 1;
	if(new_dims.width < 1)
		new_dims.width = 1;
	return {new_pos.x, new_pos.y, new_dims.width, new_dims.height};
}
