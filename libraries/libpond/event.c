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

#include "pond.h"
#include <sys/mem.h>
#include <stdlib.h>
#include <stdio.h>

PWindow** windows = NULL;
int window_count = 0;
int windows_size = 0;

PWindow* find_window(int id) {
	for(int i = 0; i < window_count; i++)
		if(windows[i]->id == id)
			return windows[i];
	return NULL;
}

void PHandleOpenWindow(socketfs_packet* packet, PEvent* event) {
	//Read the response
	PWindowOpenedPkt* resp = (struct PWindowOpenedPkt*) packet->data;
	if(!resp->successful) {
		event->window_create.window = NULL;
		return;
	}

	//Open the shared memory for the framebuffer
	struct shm shm;
	if(shmattach(resp->window.shm_id, NULL, &shm) < 0) {
		perror("libpond failed to attach window shm");
		event->window_create.window = NULL;
		return;
	}
	resp->window.buffer = shm.ptr;


	//Allocate the new window object and put it in the PEvent
	event->window_create.window = malloc(sizeof(PWindow));
	*event->window_create.window = resp->window;

	//Add the window to the array
	window_count++;
	if(window_count > windows_size) {
		// If the array size is too small to fit the new window, reallocate it with twice as much space
		// (A la std::vector)
		windows_size = window_count * 2;
		windows = realloc(windows, sizeof(PWindow*) * windows_size);
	}
	windows[window_count - 1] = event->window_create.window;
}

void PHandleDestroyWindow(socketfs_packet* packet, PEvent* event) {
	//Read the response
	PWindowDestroyedPkt* resp = (struct PWindowDestroyedPkt*) packet->data;
	event->window_destroy.successful = resp->successful;

	//Remove the window from the array
	uint8_t removing = 0;
	for(int i = 0; i < window_count; i++) {
		if(removing) {
			windows[i - 1] = windows[i];
		} else {
			if(windows[i]->id == event->window_destroy.id)
				removing = 1;
		}
	}
	window_count--;
}

void PHandleMoveWindow(socketfs_packet* packet, PEvent* event) {
	//Read the response
	PWindowMovedPkt* resp = (struct PWindowMovedPkt*) packet->data;

	//Find the window and update the event & window
	PWindow* window = find_window(resp->window_id);
	if(window) {
		event->window_move.window = window;
		event->window_move.old_x = window->x;
		event->window_move.old_y = window->y;
		window->x = resp->x;
		window->y = resp->y;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "Could not find window for window move event!\n");
	}
}

void PHandleResizeWindow(socketfs_packet* packet, PEvent* event) {
	//Read the response
	PWindowResizedPkt* resp = (struct PWindowResizedPkt*) packet->data;

	//Find the window and update the event & window
	PWindow* window = find_window(resp->window_id);
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
			window->buffer = shm.ptr;
		}
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "Could not find window for window resize event!\n");
	}
}

void PHandleMouseMove(socketfs_packet* packet, PEvent* event) {
	//Read the response
	PMouseMovePkt* resp = (struct PMouseMovePkt*) packet->data;

	//Find the window and update the event & window
	PWindow* window = find_window(resp->window_id);
	if(window) {
		event->mouse.window = window;
		event->mouse.old_buttons = window->mouse_buttons;
		event->mouse.old_x = window->mouse_x;
		event->mouse.old_y = window->mouse_y;
		window->mouse_x = resp->x;
		window->mouse_y = resp->y;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "Could not find window for window mouse move event!\n");
	}
}

void PHandleMouseButton(socketfs_packet* packet, PEvent* event) {
	//Read the response
	PMouseButtonPkt* resp = (struct PMouseButtonPkt*) packet->data;

	//Find the window and update the event & window
	PWindow* window = find_window(resp->window_id);
	if(window) {
		event->mouse.window = window;
		event->mouse.old_buttons = window->mouse_buttons;
		event->mouse.old_x = -1;
		event->mouse.old_y = -1;
		window->mouse_buttons = resp->buttons;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "Could not find window for window mouse button event!\n");
	}
}

void PHandleKeyEvent(socketfs_packet* packet, PEvent* event) {
	//Read the response
	PKeyEventPkt* resp = (struct PKeyEventPkt*) packet->data;

	//Find the window and update the event & window
	PWindow* window = find_window(resp->window_id);
	if(window) {
		event->key.window = window;
		event->key.character = resp->character;
		event->key.flags = resp->flags;
		event->key.key = resp->key;
		event->key.scancode = resp->scancode;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "Could not find window for window key event!\n");
	}
}