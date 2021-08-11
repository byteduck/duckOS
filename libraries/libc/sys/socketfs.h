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

#ifndef DUCKOS_LIBC_SOCKETFS_H
#define DUCKOS_LIBC_SOCKETFS_H

#include <sys/types.h>
#include <kernel/filesystem/socketfs/socketfs_defines.h>

struct socketfs_packet {
	int type;
	union {
		sockid_t sender;
		sockid_t recipient;
		sockid_t connected_id;
		sockid_t disconnected_id;
	};
	union {
		pid_t sender_pid;
		pid_t connected_pid;
		pid_t disconnected_pid;
	};
	size_t length;
	uint8_t data[];
};

#ifdef __cplusplus
	typedef socketfs_packet SocketFSPacket;
#endif

__DECL_BEGIN

struct socketfs_packet* read_packet(int fd);

int write_packet_of_type(int fd, int type, sockid_t id, size_t length, void* data);

inline int write_packet(int fd, sockid_t id, size_t length, void* data) {
	return write_packet_of_type(fd, SOCKETFS_TYPE_MSG, id, length, data);
}

inline int write_packet_to_host(int fd, size_t length, void* data) {
	return write_packet_of_type(fd, SOCKETFS_TYPE_MSG, SOCKETFS_RECIPIENT_HOST, length, data);
}

__DECL_END

#endif //DUCKOS_LIBC_SOCKETFS_H
