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

#include <sys/socketfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

socketfs_packet* read_packet(int fd) {
	socketfs_packet packet_header;
	ssize_t nread = read(fd, &packet_header, sizeof(struct socketfs_packet));
	if(!nread || nread < 0) return NULL;

	socketfs_packet* ret = malloc(sizeof(socketfs_packet) + packet_header.length);
	ret->id = packet_header.id;
	ret->pid = packet_header.pid;
	ret->length = packet_header.length;
	nread = read(fd, ret->data, packet_header.length);

	if(nread < 0) {
		free(ret);
		return NULL;
	}

	return ret;
}

int write_packet(int fd, int id, size_t length, void* data) {
	socketfs_packet* packet = malloc(sizeof(struct socketfs_packet) + length);
	packet->id = id;
	packet->length = length;
	memcpy(packet->data, data, length);
	int ret = write(fd, packet, length);
	free(packet);
	return ret;
}