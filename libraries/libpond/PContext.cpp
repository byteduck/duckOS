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

#include "PContext.h"
#include <sys/socketfs.h>
#include <poll.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sys/mem.h>

PContext::PContext(int _fd): fd(_fd) {}

PContext* PContext::init() {
	int fd = open("/sock/pond", O_RDWR);
	if(fd < 0) {
		perror("libpond: Failed to open socket");
		return nullptr;
	}
	return new PContext(fd);
}

bool PContext::has_event() {
	struct pollfd pfd = {fd, POLLIN, 0};
	poll(&pfd, 1, 0);
	return pfd.revents & POLLIN;
}

PEvent PContext::next_event() {
	//Wait for the socket to be ready for reading
	struct pollfd pfd = {fd, POLLIN, 0};
	poll(&pfd, 1, -1);

	//Read the packet
	socketfs_packet* packet = read_packet(fd);
	if(!packet) {
		perror("libpond failed to read packet");
		PEvent ret = {.type = PPKT_ERROR};
		return ret;
	}

	//Make sure the packet has an ID
	if(packet->length < sizeof(short)) {
		fprintf(stderr, "libpond: Packet was too small!");
		PEvent ret = {.type = PPKT_ERROR};
		return ret;
	}

	//Get the packet type and handle it
	PEvent ret = {.type = PEVENT_UNKNOWN};
	short packet_type = *((short*)packet->data);
	switch(packet_type) {
		case PPKT_WINDOW_OPENED:
			ret.type = PEVENT_WINDOW_CREATE;
			handle_open_window(packet, &ret);
			break;
		case PPKT_WINDOW_DESTROYED:
			ret.type = PEVENT_WINDOW_DESTROY;
			handle_destroy_window(packet, &ret);
			break;
		case PPKT_WINDOW_MOVED:
			ret.type = PEVENT_WINDOW_MOVE;
			handle_move_window(packet, &ret);
			break;
		case PPKT_WINDOW_RESIZED:
			ret.type = PEVENT_WINDOW_RESIZE;
			handle_resize_window(packet, &ret);
			break;
		case PPKT_MOUSE_MOVE:
			ret.type = PEVENT_MOUSE;
			handle_mouse_move(packet, &ret);
			break;
		case PPKT_MOUSE_BUTTON:
			ret.type = PEVENT_MOUSE;
			handle_mouse_button(packet, &ret);
			break;
		case PPKT_KEY_EVENT:
			ret.type = PEVENT_KEY;
			handle_key(packet, &ret);
			break;
		default:
			break;
	}

	free(packet);
	return ret;
}

PWindow* PContext::create_window(PWindow* parent, int x, int y, int width, int height) {
	//Write the packet
	if(!send_packet(POpenWindowPkt(parent ? parent->id : 0, width, height, x, y)))
		perror("Pond: Failed to write packet");

	//Wait for the response
	PEvent event = next_event();
	if(event.type != PEVENT_WINDOW_CREATE)
		return NULL;

	//Return the event's window
	return event.window_create.window;
}

void PContext::handle_open_window(socketfs_packet* packet, PEvent* event) {
	//Read the response
	auto* resp = (struct PWindowOpenedPkt*) packet->data;
	if(resp->window_id < 0) {
		event->window_create.window = NULL;
		return;
	}

	//Open the shared memory for the framebuffer
	struct shm shm;
	if(shmattach(resp->shm_id, NULL, &shm) < 0) {
		perror("libpond: Failed to attach window shm");
		event->window_create.window = NULL;
		return;
	}

	//Allocate the new window object and put it in the PEvent
	auto* window = new PWindow {
			.id = resp->window_id,
			.width = resp->width,
			.height = resp->height,
			.x = resp->x,
			.y = resp->y,
			.shm_id = resp->shm_id,
			.buffer = (uint32_t*) shm.ptr,
			.context = this,
	};
	event->window_create.window = window;

	//Add the window to the map
	windows[window->id] = window;
}

void PContext::handle_destroy_window(socketfs_packet* packet, PEvent* event) {
	//Read the response
	auto* resp = (struct PWindowDestroyedPkt*) packet->data;
	event->window_destroy.id = resp->window_id;

	//Remove the window from the array
	auto window_pair = windows.find(resp->window_id);
	if(window_pair != windows.end())
		windows.erase(window_pair);
	else
		fprintf(stderr, "libpond: Failed to find window with id %d in map for removal\n", resp->window_id);
}

void PContext::handle_move_window(socketfs_packet* packet, PEvent* event) {
	//Read the response
	auto* resp = (struct PWindowMovedPkt*) packet->data;

	//Find the window and update the event & window
	PWindow* window = windows[resp->window_id];
	if(window) {
		event->window_move.window = window;
		event->window_move.old_x = window->x;
		event->window_move.old_y = window->y;
		window->x = resp->x;
		window->y = resp->y;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for window move event!\n");
	}
}

void PContext::handle_resize_window(socketfs_packet* packet, PEvent* event) {
	//Read the response
	auto* resp = (struct PWindowResizedPkt*) packet->data;

	//Find the window and update the event & window
	PWindow* window = windows[resp->window_id];
	if(window) {
		event->window_resize.window = window;
		event->window_resize.old_width = window->width;
		event->window_resize.old_height = window->height;
		window->width = resp->width;
		window->height = resp->height;
		//Open the new shared memory for the framebuffer if necessary
		if(resp->shm_id != window->shm_id) {
			shmdetach(window->shm_id);
			struct shm shm;
			if(shmattach(window->shm_id, NULL, &shm) < 0) {
				perror("libpond failed to attach window shm");
				event->window_create.window = NULL;
				return;
			}
			window->buffer = (uint32_t*) shm.ptr;
		}
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for window resize event!\n");
	}
}

void PContext::handle_mouse_move(socketfs_packet* packet, PEvent* event) {
	//Read the response
	auto* resp = (struct PMouseMovePkt*) packet->data;

	//Find the window and update the event & window
	PWindow* window = windows[resp->window_id];
	if(window) {
		event->mouse.window = window;
		event->mouse.old_buttons = window->mouse_buttons;
		event->mouse.old_x = window->mouse_x;
		event->mouse.old_y = window->mouse_y;
		window->mouse_x = resp->x;
		window->mouse_y = resp->y;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for window mouse move event!\n");
	}
}

void PContext::handle_mouse_button(socketfs_packet* packet, PEvent* event) {
	//Read the response
	auto* resp = (struct PMouseButtonPkt*) packet->data;

	//Find the window and update the event & window
	PWindow* window = windows[resp->window_id];
	if(window) {
		event->mouse.window = window;
		event->mouse.old_buttons = window->mouse_buttons;
		event->mouse.old_x = -1;
		event->mouse.old_y = -1;
		window->mouse_buttons = resp->buttons;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for window mouse button event!\n");
	}
}

void PContext::handle_key(socketfs_packet* packet, PEvent* event) {
	//Read the response
	auto* resp = (struct PKeyEventPkt*) packet->data;

	//Find the window and update the event & window
	PWindow* window = windows[resp->window_id];
	if(window) {
		event->key.window = window;
		event->key.character = resp->character;
		event->key.modifiers = resp->modifiers;
		event->key.key = resp->key;
		event->key.scancode = resp->scancode;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for window key event!\n");
	}
}
