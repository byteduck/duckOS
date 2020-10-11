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

#include <common/defines.h>
#include "SocketFS.h"

SocketFS::SocketFS() {
	root_entry = DC::make_shared<SocketFSInode>(*this, nullptr, 1, "", 0777u | MODE_DIRECTORY, 0, 0);
}

ino_t SocketFS::get_inode_id(pid_t pid, uint16_t fileno) {
	return (ino_t)(((unsigned) pid & 0xFFFFu) << 16u) + fileno;
}

pid_t SocketFS::get_pid(ino_t inode) {
	return (pid_t) ((inode >> 16u) & 0xFFFFu);
}

uint16_t SocketFS::get_fileno(ino_t inode) {
	return (uint16_t) (inode & 0xFFFFu);
}

char* SocketFS::name() {
	return "socketfs";
}

ResultRet<DC::shared_ptr<Inode>> SocketFS::get_inode(ino_t id) {
	if(id == 1)
		return static_cast<DC::shared_ptr<Inode>>(root_entry);

	LOCK(lock);
	for(size_t i = 0; i < sockets.size(); i++) {
		if(sockets[i]->id == id)
			return static_cast<DC::shared_ptr<Inode>>(sockets[i]);
	}

	return -ENOENT;
}

ino_t SocketFS::root_inode_id() {
	return 1;
}

uint8_t SocketFS::fsid() {
	return SOCKETFS_FSID;
}
