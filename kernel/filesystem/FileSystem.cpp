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

#include <kernel/filesystem/FileSystem.h>
#include "LinkedInode.h"

Filesystem::Filesystem(const DC::shared_ptr<FileDescriptor>& file): _file(file) {

}

char* Filesystem::name() {
	return nullptr;
}

bool Filesystem::probe(DC::shared_ptr<FileDescriptor> dev) {
	return false;
}

DC::shared_ptr<Inode> Filesystem::get_inode(InodeID iid) {
	return DC::shared_ptr<Inode>(get_inode_rawptr(iid));
}

Inode *Filesystem::get_inode_rawptr(InodeID iid) {
	return nullptr;
}

InodeID Filesystem::root_inode() {
	return root_inode_id;
}

DC::shared_ptr<FileDescriptor> Filesystem::file_descriptor() {
	return _file;
}

uint8_t Filesystem::fsid() {
	return _fsid;
}

size_t Filesystem::block_size() {
	return _block_size;
}

void Filesystem::set_block_size(size_t block_size) {
	_block_size = block_size;
}
