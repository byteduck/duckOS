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
#include "Display.h"
#include "FontManager.h"
#include <libpond/packet.h>

Client::Client(int socketfs_fd, pid_t pid): socketfs_fd(socketfs_fd), pid(pid) {

}

Client::~Client() {
	disconnected = true;
	std::vector<Window*> to_delete;
	for(auto& window : windows) {
		if(window.second->parent()) {
			if(windows.find(window.second->parent()->id()) != windows.end())
				continue; //Don't remove a window if its parent is owned by this client; deleting the parent deletes all children.
		}
		to_delete.push_back(window.second);
	}

	for(auto& window : to_delete) {
		delete window;
	}
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
		case PPKT_GET_FONT:
			get_font(packet);
			break;
		case PPKT_SET_TITLE:
			set_title(packet);
			break;
		case PPKT_REPARENT:
			reparent(packet);
			break;
		case PPKT_WINDOW_HINT:
			set_hint(packet);
			break;
		case PPKT_WINDOW_TO_FRONT:
			bring_to_front(packet);
			break;
		default:
			fprintf(stderr, "Invalid packet sent by client %d\n", pid);
			return;
	}
}

void Client::mouse_moved(Window* window, Point delta, Point relative_pos, Point absolute_pos) {
	PMouseMovePkt pkt {window->id(), delta, relative_pos, absolute_pos};
	if(write_packet(socketfs_fd, pid, sizeof(PMouseMovePkt), &pkt) < 0)
		perror("Failed to write mouse movement packet to client");
}

void Client::mouse_buttons_changed(Window* window, uint8_t new_buttons) {
	PMouseButtonPkt pkt {window->id(), new_buttons};
	if(write_packet(socketfs_fd, pid, sizeof(PMouseButtonPkt), &pkt) < 0)
		perror("Failed to write mouse button packet to client");
}

void Client::mouse_scrolled(Window* window, int scroll) {
	PMouseScrollPkt pkt {window->id(), scroll};
	if(write_packet(socketfs_fd, pid, sizeof(PMouseScrollPkt), &pkt) < 0)
		perror("Failed to write mouse scroll packet to client");
}

void Client::mouse_left(Window* window) {
	PMouseLeavePkt pkt {window->id()};
	if(write_packet(socketfs_fd, pid, sizeof(PMouseLeavePkt), &pkt) < 0)
		perror("Failed to write mouse left packet to client");
}

void Client::keyboard_event(Window* window, const KeyboardEvent& event) {
	PKeyEventPkt pkt {window->id(), event.scancode, event.key, event.character, event.modifiers};
	if(write_packet(socketfs_fd, pid, sizeof(PKeyEventPkt), &pkt) < 0)
		perror("Failed to write keyboard event packet to client");
}

void Client::window_destroyed(Window* window) {
	//Remove the window from the windows map
	windows.erase(window->id());

	//Don't try to send packets to the client; they disconnected
	if(disconnected)
		return;

	PWindowDestroyedPkt pkt {window->id()};
	if(write_packet(socketfs_fd, pid, sizeof(PWindowDestroyedPkt), &pkt) < 0)
		perror("Failed to write window destroyed packet to client");
}

void Client::window_moved(Window *window) {
    PWindowMovedPkt pkt {window->id(), window->rect().position()};
    if(write_packet(socketfs_fd, pid, sizeof(PWindowMovedPkt), &pkt) < 0)
        perror("Failed to write window movement packet to client");
}

void Client::window_resized(Window *window) {
    shmallow(window->framebuffer_shm().id, pid, SHM_WRITE | SHM_READ);
    PWindowResizedPkt pkt {window->id(), window->rect(), window->framebuffer_shm().id};
    if(write_packet(socketfs_fd, pid, sizeof(PWindowResizedPkt), &pkt) < 0)
        perror("Failed to write window resized packet to client");
}

void Client::open_window(socketfs_packet* packet) {
	if(packet->length != sizeof(POpenWindowPkt))
		return;

	auto* params = (POpenWindowPkt*) packet->data;

	Window* window;

	if(!params->parent) {
		window = new Window(Display::inst().root_window(), params->rect, params->hidden);
	} else {
		auto parent_window = windows.find(params->parent);
		if(parent_window == windows.end()) {
			//Write a non-successful response; the parent window couldn't be found
			PWindowOpenedPkt response(-1);
			write_packet(socketfs_fd, pid, sizeof(PWindowOpenedPkt), &response);
			return;
		} else {
			//Make the window with the requested parent
			window = new Window(parent_window->second, params->rect, params->hidden);
		}
	}

	window->set_client(this);
	windows.insert(std::make_pair(window->id(), window));

	//Allow the client access to the window shm
	shmallow(window->framebuffer_shm().id, pid, SHM_WRITE | SHM_READ);

	//Write the response packet
	Rect wrect = window->rect();
	PWindowOpenedPkt resp {window->id(), wrect, window->framebuffer_shm().id};
	resp._PACKET_ID = PPKT_WINDOW_OPENED;
	if(write_packet(socketfs_fd, pid, sizeof(PWindowOpenedPkt), &resp) < 0)
		perror("Failed to write window opened packet to client");
}

void Client::destroy_window(socketfs_packet* packet) {
	if(packet->length != sizeof(PDestroyWindowPkt))
		return;

	auto* params = (PDestroyWindowPkt*) packet->data;

	//Find the window in question and remove it and its children
	auto window_pair = windows.find(params->window_id);
	if(window_pair != windows.end())
		delete window_pair->second;
}

void Client::move_window(socketfs_packet* packet) {
	if(packet->length != sizeof(PMoveWindowPkt))
		return;

	auto* params = (PMoveWindowPkt*) packet->data;
	auto window_pair = windows.find(params->window_id);
	if(window_pair != windows.end()) {
		auto* window = window_pair->second;

		window->set_position(params->pos, false);

		PWindowMovedPkt resp {window->id(), params->pos};
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

		window->set_dimensions(params->dims, false);
		shmallow(window->framebuffer_shm().id, pid, SHM_WRITE | SHM_READ);

		PWindowResizedPkt resp {window->id(), window->rect(), window->framebuffer_shm().id};
		if(write_packet(socketfs_fd, pid, sizeof(PWindowResizedPkt), &resp) < 0)
			perror("Failed to write window resized packet to client");
	}
}

void Client::invalidate_window(socketfs_packet* packet) {
	if(packet->length != sizeof(PInvalidatePkt))
		return;

	auto* params = (PInvalidatePkt*) packet->data;
	auto window = windows.find(params->window_id);
	if(window != windows.end()) {
		if(params->area.x < 0 || params->area.y < 0)
			window->second->invalidate();
		else
			window->second->invalidate(params->area);
	}

}

void Client::get_font(socketfs_packet* packet) {
	if(packet->length != sizeof(PGetFontPkt))
		return;

	auto* params = (PGetFontPkt*) packet->data;
	params->font_name[(sizeof(params->font_name) / sizeof(*params->font_name)) - 1] = '\0'; //Make sure we don't overflow
	auto* font = FontManager::inst().get_font(params->font_name);

	if(font) {
		if(shmallow(font->shm_id(), pid, SHM_READ) < 0) {
			perror("Failed to grant client access to font shm");
			font = nullptr;
		}
	}

	PFontResponsePkt pkt(font ? font->shm_id() : -1);
	if(!send_packet(pkt))
		perror("Failed to write font response packet to client");
}

void Client::set_title(socketfs_packet* packet) {
	if(packet->length != sizeof(PSetTitlePkt))
		return;

	auto* params = (PSetTitlePkt*) packet->data;
	params->title[(sizeof(params->title) / sizeof(*params->title)) - 1] = '\0'; //Make sure we don't overflow

	auto* window = windows[params->window_id];
	if(window)
		windows[params->window_id]->set_title(params->title);
}

void Client::reparent(socketfs_packet* packet) {
	if(packet->length != sizeof(PReparentPkt))
		return;

	auto* params = (PReparentPkt*) packet->data;
	auto* window = windows[params->window_id];
	if(window)
		window->reparent(windows[params->parent_id]);
}

void Client::set_hint(socketfs_packet* packet) {
	if(packet->length != sizeof(PWindowHintPkt))
		return;

	auto* params = (PWindowHintPkt*) packet->data;
	auto* window = windows[params->window_id];
	if(window)
		window->set_hint(params->hint, params->value);
}

void Client::bring_to_front(socketfs_packet* packet) {
	if(packet->length != sizeof(PWindowToFrontPkt))
		return;

	auto* params = (PWindowToFrontPkt*) packet->data;
	auto* window = windows[params->window_id];
	if(window)
		window->move_to_front();
}
