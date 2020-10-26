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
#include <sys/socketfs.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int pond_fd = -1;

int PInit() {
	pond_fd = open("/sock/pond", O_RDWR);
	if(pond_fd < 0) {
		perror("libpond failed to open socket");
		return -1;
	}
	return 0;
}

PEvent PNextEvent() {
	//Wait for the socket to be ready for reading
	struct pollfd pfd = {pond_fd, POLLIN, 0};
	poll(&pfd, 1, -1);

	//Read the packet
	socketfs_packet* packet = read_packet(pond_fd);
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
			PHandleOpenWindow(packet, &ret);
			break;
		case PPKT_WINDOW_DESTROYED:
			ret.type = PEVENT_WINDOW_DESTROY;
			PHandleDestroyWindow(packet, &ret);
			break;
		case PPKT_WINDOW_MOVED:
			ret.type = PEVENT_WINDOW_MOVE;
			PHandleMoveWindow(packet, &ret);
			break;
		case PPKT_WINDOW_RESIZED:
			ret.type = PEVENT_WINDOW_RESIZE;
			PHandleResizeWindow(packet, &ret);
			break;
		case PPKT_MOUSE_MOVE:
			ret.type = PEVENT_MOUSE;
			PHandleMouseMove(packet, &ret);
			break;
		case PPKT_MOUSE_BUTTON:
			ret.type = PEVENT_MOUSE;
			PHandleMouseButton(packet, &ret);
			break;
		case PPKT_KEY_EVENT:
			ret.type = PEVENT_KEY;
			PHandleKeyEvent(packet, &ret);
			break;
		default:
			break;
	}
	free(packet);
	return ret;
}

PWindow* PCreateWindow(PWindow* parent, int x, int y, int width, int height) {
	//Write the packet
	POpenWindowPkt pkt;
	pkt._PACKET_ID = PPKT_OPEN_WINDOW;
	pkt.parent = parent ? parent->id : 0;
	pkt.width = width;
	pkt.height = height;
	pkt.x = x;
	pkt.y = y;
	if(write_packet(pond_fd, SOCKETFS_HOST, sizeof(POpenWindowPkt), &pkt) < 0)
		perror("Pond: Failed to write packet");

	//Wait for the response
	PEvent event = PNextEvent();
	if(event.type != PEVENT_WINDOW_CREATE)
		return NULL;

	//Return the event's window
	return event.window_create.window;
}

int PDestroyWindow(PWindow* window) {
	//Write the packet
	PDestroyWindowPkt pkt;
	pkt._PACKET_ID = PPKT_OPEN_WINDOW;
	pkt.window_id = window->id;
	if(write_packet(pond_fd, SOCKETFS_HOST, sizeof(PDestroyWindowPkt), &pkt) < 0)
		perror("Pond: Failed to write packet");

	//Wait for the response
	PEvent event = PNextEvent();
	if(event.type != PEVENT_WINDOW_DESTROY)
		return -1;

	return event.window_destroy.successful;
}

void PInvalidateWindow(PWindow* window) {
	PInvalidatePkt pkt = {PPKT_INVALIDATE_WINDOW, window->id, -1, -1, -1, -1};
	if(write_packet(pond_fd, SOCKETFS_HOST, sizeof(PInvalidatePkt), &pkt) < 0)
		perror("Pond: Failed to write packet");
}

void PInvalidateWindowArea(PWindow* window, int x, int y, int width, int height) {
	PInvalidatePkt pkt = {PPKT_INVALIDATE_WINDOW, window->id, x, y, width, height};
	if(write_packet(pond_fd, SOCKETFS_HOST, sizeof(PInvalidatePkt), &pkt) < 0)
		perror("Pond: Failed to write packet");
}