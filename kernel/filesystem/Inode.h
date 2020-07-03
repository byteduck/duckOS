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

#ifndef DUCKOS_INODE_H
#define DUCKOS_INODE_H

#include <kernel/kstddef.h>
#include <common/shared_ptr.hpp>
#include <common/string.h>
#include "InodeMetadata.h"
#include "DirectoryEntry.h"

class Filesystem;

class Inode {
public:
	ino_t id;
	Filesystem& fs;

    Inode(Filesystem& fs, ino_t id);

	virtual DC::shared_ptr<Inode> find(DC::string name);
	virtual Inode* find_rawptr(DC::string name);
	virtual ssize_t read(size_t start, size_t length, uint8_t *buffer) = 0;
	virtual ssize_t read_dir_entry(size_t start, DirectoryEntry* buffer) = 0;

	InodeMetadata metadata();

protected:
	InodeMetadata _metadata;
};


#endif //DUCKOS_INODE_H
