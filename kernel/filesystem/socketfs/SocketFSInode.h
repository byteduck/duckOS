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

#ifndef DUCKOS_SOCKETFSINODE_H
#define DUCKOS_SOCKETFSINODE_H

#include <kernel/tasking/Process.h>
#include "SocketFS.h"
#include "SocketFSClient.h"

#define SOCKETFS_CDIR_ENTRY_SIZE (sizeof(DirectoryEntry::id) + sizeof(DirectoryEntry::type) + sizeof(DirectoryEntry::name_length) + sizeof(char))
#define SOCKETFS_PDIR_ENTRY_SIZE (sizeof(DirectoryEntry::id) + sizeof(DirectoryEntry::type) + sizeof(DirectoryEntry::name_length) + sizeof(char) * 2)

class SocketFS;
class SocketFSInode: public Inode {
public:
	//SocketFSInode
	SocketFSInode(SocketFS& fs, Process* owner, ino_t id, const DC::string& name, mode_t mode, uid_t uid, gid_t gid);
	~SocketFSInode();

	//Inode
	InodeMetadata metadata() override;
	ino_t find_id(const DC::string& name) override;
	ssize_t read(size_t start, size_t length, uint8_t *buffer) override;
	ResultRet<DC::shared_ptr<LinkedInode>> resolve_link(const DC::shared_ptr<LinkedInode>& base, User& user, DC::shared_ptr<LinkedInode>* parent_storage, int options, int recursion_level) override;
	ssize_t read_dir_entry(size_t start, DirectoryEntry* buffer) override;
	ssize_t write(size_t start, size_t length, const uint8_t* buf) override;
	Result add_entry(const DC::string& name, Inode& inode) override;
	ResultRet<DC::shared_ptr<Inode>> create_entry(const DC::string& name, mode_t mode, uid_t uid, gid_t gid) override;
	Result remove_entry(const DC::string& name) override;
	Result truncate(off_t length) override;
	Result chmod(mode_t mode) override;
	Result chown(uid_t uid, gid_t gid) override;
	void open(FileDescriptor& fd, int options) override;
	void close(FileDescriptor& fd) override;

	Process* owner;
	SocketFS& fs;
	ino_t id;
	DC::string name;

private:
	static Result write_packet(SocketFSClient& client, pid_t pid, size_t size, const void* buffer);

	DC::vector<SocketFSClient> clients;
	SocketFSClient host;
	SpinLock lock;
	DirectoryEntry dir_entry;
	bool is_open = true;
};


#endif //DUCKOS_SOCKETFSINODE_H
