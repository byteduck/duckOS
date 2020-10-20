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
#include <sys/mem.h>

int pond_fd = -1;

int PInit() {
	pond_fd = open("/sock/pond", O_RDWR);
	if(pond_fd < 0) {
		perror("libpond failed to open socket");
		return -1;
	}
	return 0;
}

socketfs_packet* prev_packet = NULL;
PEvent PNextEvent() {
	//Free the previous packet if necessary
	if(prev_packet)
		free(prev_packet);

	//Wait for the socket to be ready for reading
	struct pollfd pfd = {pond_fd, POLLIN, 0};
	poll(&pfd, 1, -1);

	//Read the packet
	socketfs_packet* packet = read_packet(pond_fd);
	if(!packet) {
		perror("libpond failed to read packet");
		PEvent ret = {POND_ERROR, NULL};
		return ret;
	}

	//Make sure the packet has an ID
	if(packet->length < sizeof(short)) {
		fprintf(stderr, "libpond: Packet was too small!");
		PEvent ret = {POND_ERROR, NULL};
		return ret;
	}

	//Get the packet type, free it, and return
	short packet_type = *((short*)packet->data);
	PEvent ret = {packet_type, packet};
	return ret;
}


PWindow* PCreateWindow(PWindow* parent, int x, int y, int width, int height) {
	//Write the packet
	POpenWindowPkt pkt;
	pkt._PACKET_ID = POND_OPEN_WINDOW;
	pkt.parent = parent ? parent->id : 0;
	pkt.width = width;
	pkt.height = height;
	pkt.x = x;
	pkt.y = y;
	write_packet(pond_fd, SOCKETFS_HOST, sizeof(POpenWindowPkt), &pkt);

	//Wait for the response
	PEvent event = PNextEvent();
	if(event.id != POND_OPEN_WINDOW_RESP)
		return NULL;

	//Read the response
	POpenWindowRsp* resp = (struct POpenWindowRsp*) event.packet->data;
	if(!resp->successful)
		return NULL;

	//Open the shared memory for the framebuffer
	struct shm shm;
	if(shmattach(resp->window.shm_id, NULL, &shm) < 0) {
		perror("libpond failed to attach window shm");
		return NULL;
	}
	resp->window.buffer = shm.ptr;


	//Make the window to be returned
	PWindow* ret = malloc(sizeof(PWindow));
	*ret = resp->window;
	return ret;
}

int PCloseWindow(PWindow* window) {
	return -1;
}

void PInvalidateWindow(PWindow* window) {
	PInvalidatePkt pkt = {POND_INVALIDATE, window->id};
	write_packet(pond_fd, SOCKETFS_HOST, sizeof(PInvalidatePkt), &pkt);
}