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

#include <unistd.h>
#include "Mouse.h"
#include "Display.h"
#include <libgraphics/Image.h>
#include <libgraphics/png.h>

using namespace Gfx;

Mouse::Mouse(Window* parent): Window(parent, {0, 0, 1, 1}, false) {
	display()->set_mouse_window(this);

	mouse_fd = open("/dev/input/mouse", O_RDONLY | O_CLOEXEC);
	if(mouse_fd < 0) {
		perror("Failed to open mouse");
		return;
	}

	load_cursor(cursor_normal, "cursor.png");
	load_cursor(cursor_resize_v, "resize_v.png");
    load_cursor(cursor_resize_h, "resize_h.png");
    load_cursor(cursor_resize_dr, "resize_dr.png");
    load_cursor(cursor_resize_dl, "resize_dl.png");
	set_cursor(Pond::NORMAL);
}

int Mouse::fd() {
	return mouse_fd;
}

bool Mouse::update() {
	MouseEvent events[32];
	ssize_t nread = read(mouse_fd, &events, sizeof(MouseEvent) * 32);
	if(!nread) return false;
	int num_events = (int) nread / sizeof(MouseEvent);

	for(int i = 0; i < num_events; i++) {
		Point new_pos = rect().position();
		new_pos.x += events[i].x;
		new_pos.y -= events[i].y;
		new_pos = new_pos.constrain(parent()->rect());
		Point delta_pos = new_pos - rect().position();
		set_position(new_pos);
		_mouse_buttons = events[i].buttons;
		Display::inst().create_mouse_events(delta_pos.x, delta_pos.y, events[i].z, _mouse_buttons);
	}

	return true;
}

void Mouse::set_cursor(Pond::CursorType cursor) {
    current_type = cursor;
    Framebuffer* cursor_image;
    switch(cursor) {
        case Pond::NORMAL:
            cursor_image = cursor_normal;
            break;
        case Pond::RESIZE_H:
            cursor_image = cursor_resize_h;
            break;
        case Pond::RESIZE_V:
            cursor_image = cursor_resize_v;
            break;
        case Pond::RESIZE_DR:
            cursor_image = cursor_resize_dr;
            break;
        case Pond::RESIZE_DL:
            cursor_image = cursor_resize_dl;
            break;
        default:
            cursor_image = cursor_normal;
    }
    if(!cursor_image)
        return;

    set_dimensions({cursor_image->width, cursor_image->height});
    _framebuffer.copy({cursor_image->data, cursor_image->width, cursor_image->height}, {0,0, cursor_image->width, cursor_image->height} ,{0,0});
}

void Mouse::load_cursor(Image*& storage, const std::string& filename) {
    FILE* cursor = fopen((std::string("/usr/share/cursors/") + filename).c_str(), "re");
    if(!cursor) {
        perror("Failed to open cursor icon");
        return;
    }

    Image* cursor_image = load_png_from_file(cursor);
    fclose(cursor);
    if(!cursor_image) {
        perror("Failed to load cursor icon");
        return;
    }

    storage = cursor_image;
}
