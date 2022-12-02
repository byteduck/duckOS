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

#include <kernel/filesystem/Inode.h>
#include <kernel/filesystem/DirectoryEntry.h>

#define PTYFS_CDIR_ENTRY_SIZE (sizeof(DirectoryEntry::id) + sizeof(DirectoryEntry::type) + sizeof(DirectoryEntry::name_length) + sizeof(char))
#define PTYFS_PDIR_ENTRY_SIZE (sizeof(DirectoryEntry::id) + sizeof(DirectoryEntry::type) + sizeof(DirectoryEntry::name_length) + sizeof(char) * 2)

class PTYDevice;
class PTYFS;
class PTYFSInode: public Inode {
public:
	enum Type {
		ROOT = 1, PTY = 2
	};

	PTYFSInode(PTYFS& fs, Type type, const kstd::Arc<PTYDevice>& pty);
	kstd::Arc<PTYDevice> pty();

	//Inode
	InodeMetadata metadata() override;
	ino_t find_id(const kstd::string& name) override;
	ssize_t read(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) override;
	ResultRet<kstd::Arc<LinkedInode>> resolve_link(const kstd::Arc<LinkedInode>& base, const User& user, kstd::Arc<LinkedInode>* parent_storage, int options, int recursion_level) override;
	ssize_t read_dir_entry(size_t start, SafePointer<DirectoryEntry> buffer, FileDescriptor* fd) override;
	ssize_t write(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) override;
	Result add_entry(const kstd::string& name, Inode& inode) override;
	ResultRet<kstd::Arc<Inode>> create_entry(const kstd::string& name, mode_t mode, uid_t uid, gid_t gid) override;
	Result remove_entry(const kstd::string& name) override;
	Result truncate(off_t length) override;
	Result chmod(mode_t mode) override;
	Result chown(uid_t uid, gid_t gid) override;
	void open(FileDescriptor& fd, int options) override;
	void close(FileDescriptor& fd) override;
	bool can_read(const FileDescriptor& fd) override;

private:
	Type type;
	kstd::Arc<PTYDevice> _pty;
	PTYFS& ptyfs;
	DirectoryEntry dir_entry;
};


