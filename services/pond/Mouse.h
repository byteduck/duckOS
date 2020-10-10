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

#ifndef DUCKOS_MOUSE_H
#define DUCKOS_MOUSE_H

#include <cstdint>
#include <fcntl.h>
#include <cstdio>

struct MouseEvent {
	int x, y, z;
	uint8_t buttons;
};

class Mouse {
public:
	Mouse();
	int fd();
	bool update();
	void set_constraints(int min_x, int min_y, int max_x, int max_y);

	int x = 0;
	int y = 0;

private:
	int mouse_fd;
	int min_x = 0;
	int min_y = 0;
	int max_x = 0;
	int max_y = 0;
	bool inited = false;
};


#endif //DUCKOS_MOUSE_H
