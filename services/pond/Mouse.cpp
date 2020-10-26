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

Mouse::Mouse(Window* parent): Window(parent, {0, 0, 3, 3}) {
	display()->set_mouse_window(this);

	mouse_fd = open("/dev/input/mouse", O_RDONLY);
	if(mouse_fd < 0) {
		perror("Failed to open mouse");
		return;
	}

	framebuffer().fill({0, 0, 3, 3}, {255, 255, 255});
	*framebuffer().at({1, 1}) = {100, 100, 100};
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
		set_position(new_pos.constrain(parent()->rect()));
		_mouse_buttons = events[i].buttons;
		Display::inst().create_mouse_events(events[i].x, -events[i].y, events[i].buttons);
	}

	return true;
}
