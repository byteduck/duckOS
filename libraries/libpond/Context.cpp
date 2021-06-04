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

#include "Context.h"
#include "Window.h"
#include <sys/socketfs.h>
#include <poll.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sys/mem.h>
#include <libgraphics/font.h>
#include <vector>

using namespace Pond;

Context::Context(int _fd): fd(_fd) {}

Context* Context::init() {
	int fd = open("/sock/pond", O_RDWR | O_CLOEXEC);
	if(fd < 0) {
		perror("libpond: Failed to open socket");
		return nullptr;
	}
	return new Context(fd);
}

bool Context::has_event() {
	read_events(false);
	return !events.empty();
}

Event Context::next_event() {
	if(events.empty())
		read_events(true);
	else
		read_events(false);
	Event ret = events.front();
	events.pop_front();
	return ret;
}

Event Context::next_event(int type) {
	while(true) {
		if(events.empty())
			read_events(true);
		else
			read_events(false);

		auto it = events.begin();
		while(it != events.end()) {
			if(it->type == type) {
				Event ret = *it;
				events.erase(it);
				return ret;
			}
			it++;
		}
	}
}

Window* Context::create_window(Window* parent, Rect rect, bool hidden) {
	//Write the packet
	if(!send_packet(POpenWindowPkt(parent ? parent->id() : 0, rect, hidden)))
		perror("Pond: Failed to write packet");

	//Wait for the response and return the new window
	Event event = next_event(PEVENT_WINDOW_CREATE);
	return event.window_create.window;
}

Font* Context::get_font(const char* font) {
	//If we've already loaded the font, just return it
	if(fonts[font])
		return fonts[font];

	//Write the packet
	if(!send_packet(PGetFontPkt(font)))
		perror("Pond: Failed to write packet");

	//Wait for the response and add the font to the map
	Event event = next_event(PEVENT_FONT_RESPONSE);
	if(event.font_response.font)
		fonts[font] = event.font_response.font;

	return event.font_response.font;
}

int Context::connection_fd() {
	return fd;
}

void Context::read_events(bool block) {
	struct pollfd pfd = {fd, POLLIN, 0};
	poll(&pfd, 1, block ? -1 : 0);
	socketfs_packet* packet;
	while((packet = read_packet(fd))) {
		//Make sure the packet has an ID
		if(packet->length < sizeof(short)) {
			fprintf(stderr, "libpond: Packet was too small!");
			continue;
		}

		//Get the packet type and handle it
		Event evt = {.type = PEVENT_UNKNOWN};
		short packet_type = *((short*)packet->data);
		switch(packet_type) {
			case PPKT_WINDOW_OPENED:
				evt.type = PEVENT_WINDOW_CREATE;
				handle_open_window(packet, &evt);
				break;
			case PPKT_WINDOW_DESTROYED:
				evt.type = PEVENT_WINDOW_DESTROY;
				handle_destroy_window(packet, &evt);
				break;
			case PPKT_WINDOW_MOVED:
				evt.type = PEVENT_WINDOW_MOVE;
				handle_move_window(packet, &evt);
				break;
			case PPKT_WINDOW_RESIZED:
				evt.type = PEVENT_WINDOW_RESIZE;
				handle_resize_window(packet, &evt);
				break;
			case PPKT_MOUSE_MOVE:
				evt.type = PEVENT_MOUSE_MOVE;
				handle_mouse_move(packet, &evt);
				break;
			case PPKT_MOUSE_BUTTON:
				evt.type = PEVENT_MOUSE_BUTTON;
				handle_mouse_button(packet, &evt);
				break;
			case PPKT_MOUSE_SCROLL:
				evt.type = PEVENT_MOUSE_SCROLL;
				handle_mouse_scroll(packet, &evt);
				break;
			case PPKT_MOUSE_LEAVE:
				evt.type = PEVENT_MOUSE_LEAVE;
				handle_mouse_leave(packet, &evt);
				break;
			case PPKT_KEY_EVENT:
				evt.type = PEVENT_KEY;
				handle_key(packet, &evt);
				break;
			case PPKT_FONT_RESPONSE:
				evt.type = PEVENT_FONT_RESPONSE;
				handle_font_response(packet, &evt);
			default:
				break;
		}

		free(packet);
		events.push_back(evt);
	}
}

void Context::handle_open_window(socketfs_packet* packet, Event* event) {
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
	auto* window = new Window(resp->window_id, resp->rect, shm, this);
	event->window_create.window = window;

	//Add the window to the map
	windows[window->_id] = window;
}

void Context::handle_destroy_window(socketfs_packet* packet, Event* event) {
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

void Context::handle_move_window(socketfs_packet* packet, Event* event) {
	//Read the response
	auto* resp = (struct PWindowMovedPkt*) packet->data;

	//Find the window and update the event & window
	Window* window = windows[resp->window_id];
	if(window) {
		event->window_move.window = window;
		event->window_move.old_pos = window->position();
		window->_rect.set_position(resp->pos);
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for window move event!\n");
	}
}

void Context::handle_resize_window(socketfs_packet* packet, Event* event) {
	//Read the response
	auto* resp = (struct PWindowResizedPkt*) packet->data;

	//Find the window and update the event & window
	Window* window = windows[resp->window_id];
	if(window) {
		event->window_resize.window = window;
		event->window_resize.old_rect = window->_rect;
		window->_rect = resp->rect;
		//Open the new shared memory for the framebuffer if necessary
		if(resp->shm_id != window->_shm_id) {
			shmdetach(window->_shm_id);
			window->_shm_id = resp->shm_id;
			struct shm shm;
			if(shmattach(window->_shm_id, NULL, &shm) < 0) {
				perror("libpond failed to attach window shm");
				event->window_create.window = NULL;
				return;
			}
			window->_framebuffer = {(uint32_t*) shm.ptr, window->_rect.width, window->_rect.height};
		}
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for window resize event!\n");
	}
}

void Context::handle_mouse_move(socketfs_packet* packet, Event* event) {
	//Read the response
	auto* resp = (struct PMouseMovePkt*) packet->data;

	//Find the window and update the event & window
	Window* window = windows[resp->window_id];
	if(window) {
		event->mouse_move.window = window;
		event->mouse_move.delta = resp->delta;
		event->mouse_move.new_pos = resp->relative;
		event->mouse_move.abs_pos = resp->absolute;
		window->_mouse_pos = resp->relative;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for window mouse move event!\n");
	}
}

void Context::handle_mouse_button(socketfs_packet* packet, Event* event) {
	//Read the response
	auto* resp = (struct PMouseButtonPkt*) packet->data;

	//Find the window and update the event & window
	Window* window = windows[resp->window_id];
	if(window) {
		event->mouse_button.window = window;
		event->mouse_button.old_buttons = window->_mouse_buttons;
		event->mouse_button.new_buttons = resp->buttons;
		window->_mouse_buttons = resp->buttons;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for window mouse button event!\n");
	}
}

void Context::handle_mouse_scroll(socketfs_packet* packet, Event* event) {
	//Read the response
	auto* resp = (struct PMouseScrollPkt*) packet->data;

	//Find the window and update the event & window
	Window* window = windows[resp->window_id];
	if(window) {
		event->mouse_scroll.window = window;
		event->mouse_scroll.scroll = resp->scroll;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for mouse scroll event!\n");
	}
}

void Context::handle_mouse_leave(socketfs_packet* packet, Event* event) {
	//Read the response
	auto* resp = (struct PMouseLeavePkt*) packet->data;

	//Find the window and update the event & window
	Window* window = windows[resp->window_id];
	if(window) {
		event->mouse_leave.window = window;
		event->mouse_leave.last_pos = window->_mouse_pos;
	} else {
		event->type = PEVENT_UNKNOWN;
		fprintf(stderr, "libpond: Could not find window for window mouse button event!\n");
	}
}

void Context::handle_key(socketfs_packet* packet, Event* event) {
	//Read the response
	auto* resp = (struct PKeyEventPkt*) packet->data;

	//Find the window and update the event & window
	Window* window = windows[resp->window_id];
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

void Context::handle_font_response(socketfs_packet* packet, Event* event) {
	//Read the response
	auto* resp = (struct PFontResponsePkt*) packet->data;

	if(resp->font_shm_id < 0) {
		event->font_response.font = nullptr;
		return;
	}

	//Try attaching the shared memory
	shm fontshm;
	if(shmattach(resp->font_shm_id, nullptr, &fontshm) < 0) {
		perror("libpond: Failed to attach font shm");
		event->font_response.font = nullptr;
		return;
	}

	//Load the font (if it errors out, a message should be printed and it will return nullptr)
	event->font_response.font = Font::load_from_shm(fontshm);
}
