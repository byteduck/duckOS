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

#ifndef VFS_H
#define VFS_H

#include <kernel/device/BlockDevice.h>
#include <kernel/filesystem/Inode.h>
#include <common/shared_ptr.hpp>
#include <kernel/filesystem/FileDescriptor.h>

class Inode;

class FileDescriptor;
class Filesystem {
public:
	Filesystem(const DC::shared_ptr<FileDescriptor>& file);

	virtual char* name();
	static bool probe(DC::shared_ptr<FileDescriptor> dev);
	virtual DC::shared_ptr<Inode> get_inode(ino_t id);
	virtual Inode* get_inode_rawptr(ino_t id);
	virtual ino_t root_inode();
	virtual uint8_t fsid();
	virtual size_t block_size();
	virtual void set_block_size(size_t block_size);
	DC::shared_ptr<FileDescriptor> file_descriptor();
protected:
	DC::shared_ptr<FileDescriptor> _file;
	uint8_t _fsid;
	ino_t root_inode_id;
private:
	size_t _block_size;
};

#endif
