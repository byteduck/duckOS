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
	auto* pond = PContext::init();
	if(!pond)
		exit(-1);

	PWindow* window = pond->create_window(nullptr, 10, 10, 100, 100);
	if(!window)
		exit(-1);

	for(size_t i = 0; i < window->width * window->height; i++)
		window->buffer[i] = RGBA(0, 0, 0, 200);
	window->invalidate();

	while(1) {
		PEvent event = pond->next_event();
		if(event.type == PEVENT_MOUSE) {
			window->buffer[window->mouse_x + window->mouse_y * window->width] = RGB(200,0,200);
			window->invalidate_area(window->mouse_x, window->mouse_y, 1, 1);
		} else if(event.type == PEVENT_KEY) {
			if(event.key.character == 'q')
				exit(0);
		} else if(event.type == PEVENT_WINDOW_DESTROY)
			break;
	}
}