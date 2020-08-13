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

#include "InodeMetadata.h"

bool InodeMetadata::is_directory() const {
	return (mode & 0xF000u) == MODE_DIRECTORY;
}

bool InodeMetadata::is_simple_file() const {
	return (mode & 0xF000u) == MODE_FILE;
}

bool InodeMetadata::exists() const {
	return inode_id != 0;
}

bool InodeMetadata::is_block_device() const {
	return (mode & 0xF000u) == MODE_BLOCK_DEVICE;
}

bool InodeMetadata::is_character_device() const {
	return (mode & 0xF000u) == MODE_CHAR_DEVICE;
}

bool InodeMetadata::is_device() const {
	return (mode & 0xF000u) == MODE_CHAR_DEVICE ||  (mode & 0xF000u) == MODE_BLOCK_DEVICE;
}

bool InodeMetadata::is_symlink() const {
	return (mode & 0xF000u) == MODE_SYMLINK;
}

void InodeMetadata::stat(struct stat *stat) {
	stat->st_mode = mode;
	stat->st_size = size;
	stat->st_ino = inode_id;
	stat->st_dev = ((dev_t)dev_major << 8u) + dev_minor;
	stat->st_uid = uid;
	stat->st_gid = gid;
	//TODO: Fill in more info
}

bool InodeMetadata::can_write(const User& user) const {
	return  user.can_override_permissions() ||
			(mode | PERM_O_W) ||
			(mode | PERM_U_W && user.uid() == uid) ||
	        (mode | PERM_G_W && user.in_group(gid));
}

bool InodeMetadata::can_execute(const User& user) const {
	return  user.can_override_permissions() ||
			(mode | PERM_O_X) ||
			(mode | PERM_U_X && user.uid() == uid) ||
			(mode | PERM_G_X && user.in_group(gid));
}

bool InodeMetadata::can_read(const User& user) const {
	return  user.can_override_permissions() ||
			(mode | PERM_O_R) ||
			(mode | PERM_U_R && user.uid() == uid) ||
			(mode | PERM_G_R && user.in_group(gid));
}
