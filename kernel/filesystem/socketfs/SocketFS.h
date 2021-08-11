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

#ifndef DUCKOS_SOCKETFS_H
#define DUCKOS_SOCKETFS_H

#include <kernel/filesystem/Filesystem.h>
#include "socketfs_defines.h"
#include <kernel/kstd/vector.hpp>
#include <kernel/tasking/SpinLock.h>

#define SOCKETFS_FSID 3

struct SocketFSPacket {
	int id;
	pid_t pid;
	size_t length;
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

	//Filesystem
	char* name() override;
	ResultRet<kstd::shared_ptr<Inode>> get_inode(ino_t id) override;
	ino_t root_inode_id() override;
	uint8_t fsid() override;

protected:
	friend class SocketFSInode;
	kstd::vector<kstd::shared_ptr<SocketFSInode>> sockets;
	kstd::shared_ptr<SocketFSInode> root_entry;
	SpinLock lock;

};


#endif //DUCKOS_SOCKETFS_H
