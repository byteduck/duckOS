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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include <sys/socketfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

struct socketfs_packet* read_packet(int fd) {
	struct socketfs_packet packet_header;
	ssize_t nread = read(fd, &packet_header, sizeof(struct socketfs_packet));
	if(!nread || nread < 0)
		return NULL;

	struct socketfs_packet* ret = malloc(sizeof(struct socketfs_packet) + packet_header.length);
	ret->type = packet_header.type;
	ret->sender = packet_header.sender;
	ret->sender_pid = packet_header.sender_pid;
	ret->length = packet_header.length;

	if(packet_header.length) {
		if(read(fd, ret->data, packet_header.length) < 0) {
			free(ret);
			return NULL;
		}
	}

	return ret;
}

int write_packet_of_type(int fd, int type, sockid_t id, size_t length, void* data) {
	struct socketfs_packet* packet = malloc(sizeof(struct socketfs_packet) + length);
	packet->type = SOCKETFS_TYPE_MSG;
	packet->recipient = id;
	packet->length = length;
	memcpy(packet->data, data, length);
	int ret = write(fd, packet, length);
	free(packet);
	return ret;
}