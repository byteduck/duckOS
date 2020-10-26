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

#include "Client.h"
#include "DecorationWindow.h"
#include "Display.h"
#include <libpond/pond.h>

Client::Client(int socketfs_fd, pid_t pid): socketfs_fd(socketfs_fd), pid(pid) {

}

void Client::handle_packet(socketfs_packet* packet) {
	if(packet->length < sizeof(short))
		return; //Doesn't even include a packet header

	short packet_type = *((short*)packet->data);
	switch(packet_type) {
		case PPKT_OPEN_WINDOW:
			open_window(packet);
			break;
		case PPKT_DESTROY_WINDOW:
			destroy_window(packet);
			break;
		case PPKT_MOVE_WINDOW:
			move_window(packet);
			break;
		case PPKT_RESIZE_WINDOW:
			resize_window(packet);
			break;
		case PPKT_INVALIDATE_WINDOW:
			invalidate_window(packet);
			break;
		default:
			fprintf(stderr, "Invalid packet sent by client %d\n", pid);
			return;
	}
}

void Client::mouse_moved(Window* window, Point new_position) {
	PMouseMovePkt pkt = {PPKT_MOUSE_MOVE, window->id(), new_position.x, new_position.y};
	if(write_packet(socketfs_fd, pid, sizeof(PMouseMovePkt), &pkt) < 0)
		perror("Failed to write mouse movement packet to client");
}

void Client::mouse_buttons_changed(Window* window, uint8_t new_buttons) {
	PMouseButtonPkt pkt = {PPKT_MOUSE_BUTTON, window->id(), new_buttons};
	if(write_packet(socketfs_fd, pid, sizeof(PMouseButtonPkt), &pkt) < 0)
		perror("Failed to write mouse button packet to client");
}

void Client::open_window(socketfs_packet* packet) {
	if(packet->length != sizeof(POpenWindowPkt))
		return;

	auto* params = (POpenWindowPkt*) packet->data;

	DecorationWindow* deco_window;

	if(!params->parent) {
		deco_window = new DecorationWindow(Display::inst().root_window(), {params->x, params->y, params->width, params->height});
	} else {
		auto parent_window = windows.find(params->parent);
		if(parent_window == windows.end()) {
			//Write a non-successful response; the parent window couldn't be found
			PWindowOpenedPkt response;
			response.successful = false;
			write_packet(socketfs_fd, pid, sizeof(PWindowOpenedPkt), &response);
			return;
		} else {
			//Make the window with the requested parent
			deco_window = new DecorationWindow(parent_window->second, {params->x, params->y, params->width, params->height});
		}
	}

	auto* window = deco_window->contents();
	window->set_client(this);
	windows.insert(std::make_pair(window->id(), window));

	//Allow the client access to the window shm
	shmallow(window->framebuffer_shm().id, pid, SHM_WRITE | SHM_READ);

	//Write the response packet
	PWindowOpenedPkt resp;
	resp._PACKET_ID = PPKT_WINDOW_OPENED;
	Rect wrect = window->rect();
	resp.window.width = wrect.width;
	resp.window.height = wrect.height;
	resp.window.x = wrect.x;
	resp.window.y = wrect.y;
	resp.window.id = window->id();
	resp.window.shm_id = window->framebuffer_shm().id;
	resp.window.buffer = nullptr;
	resp.successful = true;
	if(write_packet(socketfs_fd, pid, sizeof(PWindowOpenedPkt), &resp) < 0)
		perror("Failed to write window opened packet to client");
}

void Client::destroy_window(socketfs_packet* packet) {
	if(packet->length != sizeof(PDestroyWindowPkt))
		return;

	auto* params = (PDestroyWindowPkt*) packet->data;

	PWindowDestroyedPkt resp = {PPKT_WINDOW_DESTROYED, false};

	//Find the window in question and remove it
	auto window = windows.find(params->window_id);
	if(window != windows.end()) {
		delete window->second;
		windows.erase(window);
		resp.successful = true;
	}

	if(write_packet(socketfs_fd, pid, sizeof(PWindowDestroyedPkt), &resp) < 0)
		perror("Failed to write window destroyed packet to client");
}

void Client::move_window(socketfs_packet* packet) {
	if(packet->length != sizeof(PMoveWindowPkt))
		return;

	auto* params = (PMoveWindowPkt*) packet->data;
	auto window_pair = windows.find(params->window_id);
	if(window_pair != windows.end()) {
		auto* window = window_pair->second;

		//If this window is decorated, set the position of the decoration window instead
		if(window->parent() && window->parent()->is_decoration())
			window->parent()->set_position(Point{params->x, params->y} - window->rect().position());
		else
			window->set_position({params->x, params->y});

		PWindowMovedPkt resp = {PPKT_WINDOW_MOVED, window->id(), params->x, params->y};
		if(write_packet(socketfs_fd, pid, sizeof(PWindowMovedPkt), &resp) < 0)
			perror("Failed to write window moved packet to client");
	}
}

void Client::resize_window(socketfs_packet* packet) {
	if(packet->length != sizeof(PResizeWindowPkt))
		return;

	auto* params = (PResizeWindowPkt*) packet->data;
	auto window_pair = windows.find(params->window_id);
	if(window_pair != windows.end()) {
		auto* window = window_pair->second;

		//If this window is decorated, set the size of the decoration window as well
		if(window->parent() && window->parent()->is_decoration())
			((DecorationWindow*) window->parent())->set_content_dimensions({params->width, params->height});
		else
			window->set_dimensions({params->width, params->height});

		PWindowResizedPkt resp = {PPKT_WINDOW_MOVED, window->id(), params->width, params->height, window->framebuffer_shm().id};
		if(write_packet(socketfs_fd, pid, sizeof(PWindowMovedPkt), &resp) < 0)
			perror("Failed to write window moved packet to client");
	}
}

void Client::invalidate_window(socketfs_packet* packet) {
	if(packet->length != sizeof(PInvalidatePkt))
		return;

	auto* params = (PInvalidatePkt*) packet->data;
	auto window = windows.find(params->window_id);
	if(window != windows.end()) {
		if(params->x < 0 || params->y < 0)
			window->second->invalidate();
		else
			window->second->invalidate({params->x, params->y, params->width, params->height});
	}

}
