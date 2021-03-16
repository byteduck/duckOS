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

#include "PTYFSInode.h"
#include <kernel/terminal/PTYDevice.h>
#include <common/defines.h>
#include "PTYFS.h"
#include <kernel/terminal/PTYControllerDevice.h>

PTYFSInode::PTYFSInode(PTYFS& fs, Type type, const DC::shared_ptr<PTYDevice>& pty): Inode(fs, type == PTY ? pty->id() + 2 : 1), type(type), _pty(pty), ptyfs(fs) {
	// Inode ID 1 = Root, 2+ = (pty ID + 2)
	DC::string name;
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

DC::shared_ptr<PTYDevice> PTYFSInode::pty() {
	return _pty;
}

InodeMetadata PTYFSInode::metadata() {
	if(type != ROOT) return _metadata;

	LOCK(ptyfs._lock);
	InodeMetadata ret = _metadata;
	ret.size = PTYFS_PDIR_ENTRY_SIZE + PTYFS_CDIR_ENTRY_SIZE;
	for(size_t i = 1; i < ptyfs._entries.size(); i++)
		ret.size += ptyfs._entries[i]->dir_entry.entry_length();

	return ret;
}

ino_t PTYFSInode::find_id(const DC::string& name) {
	LOCK(ptyfs._lock);
	for(size_t i = 1; i < ptyfs._entries.size(); i++) {
		if(name == ptyfs._entries[i]->dir_entry.name)
			return ptyfs._entries[i]->id;
	}
	return 0;
}

ssize_t PTYFSInode::read(size_t start, size_t length, uint8_t* buffer, FileDescriptor* fd) {
	//Hopefully we aren't haunted by a null pointer :)
	return _pty->read(*fd, start, buffer, length);
}

ssize_t PTYFSInode::read_dir_entry(size_t start, DirectoryEntry* buffer, FileDescriptor* fd) {
	if(type != ROOT)
		return -ENOTDIR;

	if(start == 0) {
		DirectoryEntry ent(id, TYPE_DIR, ".");
		memcpy(buffer, &ent, sizeof(DirectoryEntry));
		return PTYFS_CDIR_ENTRY_SIZE;
	} else if(start == PTYFS_CDIR_ENTRY_SIZE) {
		DirectoryEntry ent(0, TYPE_DIR, "..");
		memcpy(buffer, &ent, sizeof(DirectoryEntry));
		return PTYFS_PDIR_ENTRY_SIZE;
	}

	size_t cur_index = PTYFS_CDIR_ENTRY_SIZE + PTYFS_PDIR_ENTRY_SIZE;
	LOCK(ptyfs._lock);

	for(size_t i = 1; i < ptyfs._entries.size(); i++) {
		auto& e = ptyfs._entries[i];
		if(cur_index >= start) {
			memcpy(buffer, &e->dir_entry, sizeof(DirectoryEntry));
			return e->dir_entry.entry_length();
		}
		cur_index += e->dir_entry.entry_length();
	}

	return 0;
}

ssize_t PTYFSInode::write(size_t start, size_t length, const uint8_t* buf, FileDescriptor* fd) {
	//Hopefully we aren't haunted by a null pointer :)
	return _pty->write(*fd, start, buf, length);
}

bool PTYFSInode::can_read(const FileDescriptor& fd) {
	return _pty->can_read(fd);
}

Result PTYFSInode::add_entry(const DC::string& name, Inode& inode) { return -EROFS; }
ResultRet<DC::shared_ptr<Inode>> PTYFSInode::create_entry(const DC::string& name, mode_t mode, uid_t uid, gid_t gid) { return -EROFS; }
Result PTYFSInode::remove_entry(const DC::string& name) { return -EROFS; }
Result PTYFSInode::truncate(off_t length) { return -EROFS; }
Result PTYFSInode::chmod(mode_t mode) { return -EROFS; }
Result PTYFSInode::chown(uid_t uid, gid_t gid) { return -EROFS; }
void PTYFSInode::open(FileDescriptor& fd, int options) {}
void PTYFSInode::close(FileDescriptor& fd) {}
ResultRet<DC::shared_ptr<LinkedInode>> PTYFSInode::resolve_link(const DC::shared_ptr<LinkedInode>& base, User& user,
																DC::shared_ptr<LinkedInode>* parent_storage,
																int options, int recursion_level) { return -ENOLINK; }