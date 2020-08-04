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
	//TODO: Fill in more info
}