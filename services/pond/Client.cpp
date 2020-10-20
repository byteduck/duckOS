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
		case POND_OPEN_WINDOW:
			open_window(packet);
			break;
		case POND_DESTROY_WINDOW:
			destroy_window(packet);
			break;
		case POND_MOVE_WINDOW:
			move_window(packet);
			break;
		case POND_RESIZE_WINDOW:
			resize_window(packet);
			break;
		case POND_INVALIDATE:
			invalidate_window(packet);
			break;
		default:
			fprintf(stderr, "Invalid packet sent by client %d\n", pid);
			return;
	}
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
			POpenWindowRsp response;
			response.successful = false;
			write_packet(socketfs_fd, pid, sizeof(POpenWindowRsp), &response);
			return;
		} else {
			//Make the window with the requested parent
			deco_window = new DecorationWindow(parent_window->second, {params->x, params->y, params->width, params->height});
		}
	}

	auto* window = deco_window->contents();

	int win_id = ++current_winid;
	windows.insert(std::make_pair(win_id, window));

	//Allow the client access to the window shm
	shmallow(window->framebuffer_shm().id, pid, SHM_WRITE | SHM_READ);

	//Write the response packet
	POpenWindowRsp resp;
	resp._PACKET_ID = POND_OPEN_WINDOW_RESP;
	Rect wrect = window->rect();
	resp.window.width = wrect.width;
	resp.window.height = wrect.height;
	resp.window.x = wrect.x;
	resp.window.y = wrect.y;
	resp.window.id = win_id;
	resp.window.shm_id = window->framebuffer_shm().id;
	resp.window.buffer = nullptr;
	resp.successful = true;
	if(write_packet(socketfs_fd, pid, sizeof(POpenWindowRsp), &resp) < 0)
		perror("Failed to write packet to client");
}

void Client::destroy_window(socketfs_packet* packet) {
	if(packet->length != sizeof(PDestroyWindowPkt))
		return;

	auto* params = (PDestroyWindowPkt*) packet->data;

	PDestroyWindowRsp resp = {POND_DESTROY_WINDOW_RESP,false};
	resp._PACKET_ID = POND_DESTROY_WINDOW_RESP;

	//Find the window in question and remove it
	auto window = windows.find(params->id);
	if(window != windows.end()) {
		delete window->second;
		windows.erase(window);
		resp.successful = true;
	}

	if(write_packet(socketfs_fd, pid, sizeof(PDestroyWindowRsp), &resp) < 0)
		perror("Failed to write packet to client");
}

void Client::move_window(socketfs_packet* packet) {

}

void Client::resize_window(socketfs_packet* packet) {

}

void Client::invalidate_window(socketfs_packet* packet) {
	if(packet->length != sizeof(PInvalidatePkt))
		return;

	auto* params = (PInvalidatePkt*) packet->data;
	auto window = windows.find(params->window_id);
	if(window != windows.end())
		window->second->invalidate();
}
