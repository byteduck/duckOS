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

#include "Mouse.h"
#include "Display.h"
#include "Server.h"
#include "Window.h"

int main() {
	auto* display = new Display;
	auto* server = new Server;
	auto* main_window = new Window(display);
	auto* mouse = new Mouse(main_window);

	auto* window2 = new Window(main_window, {{200, 200}, 150, 200});
	window2->framebuffer().fill({{0,0}, 150, 200}, {125, 255, 255});
	window2->framebuffer().fill({{2,2}, 146, 196}, {100, 100, 100});

	auto* window3 = new Window(main_window, {{150, 150}, 150, 100});
	window3->framebuffer().fill({{0,0}, 150, 100}, {255, 125, 255});
	window3->framebuffer().fill({{2,2}, 146, 96}, {100, 100, 100});

	window2->move_to_front();

	while(true) {
		//TODO: Implement select() and poll() in kernel so we don't take up CPU time
		if(mouse->update());
			//window2->set_position(mouse->rect().position);
		server->handle_packets();
		display->repaint();
	}
}