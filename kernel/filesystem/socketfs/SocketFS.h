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

#pragma once

#include <kernel/filesystem/Filesystem.h>
#include "socketfs_defines.h"
#include <kernel/kstd/vector.hpp>
#include <kernel/tasking/SpinLock.h>

#define SOCKETFS_FSID 3

struct SocketFSPacket {
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
	int shm_id;
	int shm_perms;
	uint8_t data[];
};

class SocketFSInode;
class SocketFS: public Filesystem {
public:
	//SocketFS
	SocketFS();
	static ino_t get_inode_id(pid_t pid, uint16_t fileno);
	static pid_t get_pid(ino_t inode);
	static uint16_t get_fileno(ino_t inode);
	static sockid_t client_hash(const void* fd_pointer);

	//Filesystem
	char* name() override;
	ResultRet<kstd::Arc<Inode>> get_inode(ino_t id) override;
	ino_t root_inode_id() override;
	uint8_t fsid() override;

protected:
	friend class SocketFSInode;
	kstd::vector<kstd::Arc<SocketFSInode>> sockets;
	kstd::Arc<SocketFSInode> root_entry;
	SpinLock lock;

};


