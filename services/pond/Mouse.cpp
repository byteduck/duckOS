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

Mouse::Mouse() {
	mouse_fd = open("/dev/input/mouse", O_RDONLY);
	if(mouse_fd < 0) {
		perror("Failed to open mouse");
		return;
	}
	inited = true;
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
		x += events[i].x;
		y -= events[i].y;
	}

	if(x < min_x)
		x = min_x;
	if(x > max_x)
		x = max_x;
	if(y < min_y)
		y = min_y;
	if(y > max_y)
		y = max_y;

	return true;
}

void Mouse::set_constraints(int min_x, int min_y, int max_x, int max_y) {
	if(min_x > max_x || min_y > max_y) return;
	this->min_x = min_x;
	this->min_y = min_y;
	this->max_x = max_x;
	this->max_y = max_y;
}
