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

#include <libpond/pond.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main() {
	if(PInit() < 0)
		exit(-1);

	PWindow* window = PCreateWindow(NULL, 10, 10, 100, 100);
	if(window == NULL)
		exit(-1);

	memset(window->buffer, 0xAA, sizeof(*window->buffer) * window->width * window->height);
	PInvalidateWindow(window);

	while(1) {
		PEvent event = PNextEvent();
		if(event.type == PEVENT_MOUSE) {
			window->buffer[window->mouse_x + window->mouse_y * window->width] = RGB(200,0,200);
			PInvalidateWindowArea(window, window->mouse_x, window->mouse_y, 1, 1);
		} else if(event.type == PEVENT_KEY) {
			if(event.key.character == 'q')
				exit(0);
		} else if(event.type == PEVENT_WINDOW_DESTROY)
			break;
	}
}