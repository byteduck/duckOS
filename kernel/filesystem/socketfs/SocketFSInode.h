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

#include "SocketFS.h"
#include "SocketFSClient.h"
#include <kernel/filesystem/Inode.h>
#include <kernel/kstd/string.h>
#include <kernel/filesystem/DirectoryEntry.h>

#define SOCKETFS_CDIR_ENTRY_SIZE (sizeof(DirectoryEntry::id) + sizeof(DirectoryEntry::type) + sizeof(DirectoryEntry::name_length) + sizeof(char))
#define SOCKETFS_PDIR_ENTRY_SIZE (sizeof(DirectoryEntry::id) + sizeof(DirectoryEntry::type) + sizeof(DirectoryEntry::name_length) + sizeof(char) * 2)

class SocketFS;
class Process;
class LinkedInode;
class InodeMetadata;
class SocketFSInode: public Inode {
public:
	//SocketFSInode
	SocketFSInode(SocketFS& fs, ino_t id, const kstd::string& name, mode_t mode, uid_t uid, gid_t gid);
	~SocketFSInode();

	//Inode
	InodeMetadata metadata() override;
	ino_t find_id(const kstd::string& name) override;
	ssize_t read(size_t start, size_t length, SafePointer<uint8_t> buf, FileDescriptor* fd) override;
	ResultRet<kstd::Arc<LinkedInode>> resolve_link(const kstd::Arc<LinkedInode>& base, const User& user, kstd::Arc<LinkedInode>* parent_storage, int options, int recursion_level) override;
	ssize_t read_dir_entry(size_t start, SafePointer<DirectoryEntry> buf, FileDescriptor* fd) override;
	ssize_t write(size_t start, size_t length, SafePointer<uint8_t> buf, FileDescriptor* fd) override;
	Result add_entry(const kstd::string& name, Inode& inode) override;
	ResultRet<kstd::Arc<Inode>> create_entry(const kstd::string& name, mode_t mode, uid_t uid, gid_t gid) override;
	Result remove_entry(const kstd::string& name) override;
	Result truncate(off_t length) override;
	Result chmod(mode_t mode) override;
	Result chown(uid_t uid, gid_t gid) override;
	void open(FileDescriptor& fd, int options) override;
	void close(FileDescriptor& fd) override;
	bool can_read(const FileDescriptor& fd) override;

	SocketFS& fs;
	ino_t id;
	kstd::string name;

private:
	Result write_packet(SocketFSClient& recipient, int type, sockid_t sender, size_t size, int shm_id, int shm_perms, SafePointer<uint8_t> buffer, bool nonblock);

	kstd::vector<SocketFSClient> clients;
	SocketFSClient host;
	SpinLock lock;
	DirectoryEntry dir_entry;
	bool is_open = true;
};


