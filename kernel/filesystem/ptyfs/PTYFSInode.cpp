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

#include "PTYFSInode.h"
#include <kernel/terminal/PTYDevice.h>
#include "PTYFS.h"
#include <kernel/terminal/PTYControllerDevice.h>
#include <kernel/kstd/cstring.h>
#include <kernel/filesystem/LinkedInode.h>

PTYFSInode::PTYFSInode(PTYFS& fs, Type type, const kstd::Arc<PTYDevice>& pty): Inode(fs, type == PTY ? pty->id() + 2 : 1), type(type), _pty(pty), ptyfs(fs) {
	// Inode ID 1 = Root, 2+ = (pty ID + 2)
	kstd::string name;
	uint8_t dtype;
	switch(type) {
		case ROOT:
			dtype = TYPE_DIR;
			name = "/";
			_metadata.mode = MODE_DIRECTORY;
			break;
		case PTY: {
			dtype = TYPE_CHARACTER_DEVICE;
			char namebuf[16];
			itoa(pty->id(), namebuf, 10);
			name = namebuf;
			_metadata.mode = MODE_CHAR_DEVICE;
			_metadata.dev_major = pty->major();
			_metadata.dev_minor = pty->minor();
			break;
		}
	}

	_metadata.uid = 0;
	_metadata.gid = 0;
	_metadata.size = 0;

	dir_entry = DirectoryEntry(id, dtype, name);
}

kstd::Arc<PTYDevice> PTYFSInode::pty() {
	return _pty;
}

InodeMetadata PTYFSInode::metadata() {
	return _metadata;
}

ino_t PTYFSInode::find_id(const kstd::string& name) {
	LOCK(ptyfs._lock);
	for(size_t i = 1; i < ptyfs._entries.size(); i++) {
		if(name == ptyfs._entries[i]->dir_entry.name)
			return ptyfs._entries[i]->id;
	}
	return 0;
}

ssize_t PTYFSInode::read(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) {
	//Hopefully we aren't haunted by a null pointer :)
	return _pty->read(*fd, start, buffer, length);
}

void PTYFSInode::iterate_entries(kstd::IterationFunc<const DirectoryEntry&> callback) {
	ASSERT(type == ROOT);
	LOCK(ptyfs._lock);
	ITER_RET(callback(DirectoryEntry(id, TYPE_DIR, ".")));
	ITER_RET(callback(DirectoryEntry(0, TYPE_DIR, "..")));
	for(auto& entry : ptyfs._entries)
		ITER_BREAK(callback(entry->dir_entry));
}

ssize_t PTYFSInode::write(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) {
	//Hopefully we aren't haunted by a null pointer :)
	return _pty->write(*fd, start, buffer, length);
}

bool PTYFSInode::can_read(const FileDescriptor& fd) {
	return _pty->can_read(fd);
}

Result PTYFSInode::add_entry(const kstd::string& name, Inode& inode) { return Result(-EROFS); }
ResultRet<kstd::Arc<Inode>> PTYFSInode::create_entry(const kstd::string& name, mode_t mode, uid_t uid, gid_t gid) { return Result(-EROFS); }
Result PTYFSInode::remove_entry(const kstd::string& name) { return Result(-EROFS); }
Result PTYFSInode::truncate(off_t length) { return Result(-EROFS); }
Result PTYFSInode::chmod(mode_t mode) { return Result(-EROFS); }
Result PTYFSInode::chown(uid_t uid, gid_t gid) { return Result(-EROFS); }
void PTYFSInode::open(FileDescriptor& fd, int options) {}
void PTYFSInode::close(FileDescriptor& fd) {}
ResultRet<kstd::Arc<LinkedInode>> PTYFSInode::resolve_link(const kstd::Arc<LinkedInode>& base, const User& user, kstd::Arc<LinkedInode>* parent_storage, int options, int recursion_level) { return Result(-ENOLINK); }