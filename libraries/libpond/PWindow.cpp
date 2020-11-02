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

#include "PWindow.h"
#include "PContext.h"
#include <cstdio>

int PWindow::destroy() {
	if(!context->send_packet(PDestroyWindowPkt(id))) {
		perror("Pond: Failed to write packet");
		return -1;
	}
	return 0;
}

void PWindow::invalidate() {
	if(!context->send_packet(PInvalidatePkt(id, -1, -1, -1, -1)))
		perror("Pond: Failed to write packet");
}

void PWindow::invalidate_area(int area_x, int area_y, int area_width, int area_height) {
	if(!context->send_packet(PInvalidatePkt(id, area_x, area_y, area_width, area_height)))
		perror("Pond: Failed to write packet");
}