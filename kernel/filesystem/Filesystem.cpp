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

#include <kernel/filesystem/Filesystem.h>
#include <common/defines.h>
#include <kernel/tasking/TaskManager.h>
#include "LinkedInode.h"

Filesystem::Filesystem(const DC::shared_ptr<FileDescriptor>& file): _file(file) {

}

char* Filesystem::name() {
	return nullptr;
}

bool Filesystem::probe(DC::shared_ptr<FileDescriptor> dev) {
	return false;
}

ResultRet<DC::shared_ptr<Inode>> Filesystem::get_inode(ino_t iid) {
	LOCK(_inode_cache_lock);
	auto inode_perhaps = get_cached_inode(iid);
	if(inode_perhaps.is_error()) {
		Inode* in = get_inode_rawptr(iid);
		if(in) {
			auto ins = DC::shared_ptr<Inode>(in);
			add_cached_inode(ins);
			return ins;
		} else return -ENOENT;
	} else {
		return inode_perhaps.value();
	}
}

Inode *Filesystem::get_inode_rawptr(ino_t iid) {
	return nullptr;
}

ino_t Filesystem::root_inode() {
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

ResultRet<DC::shared_ptr<Inode>> Filesystem::get_cached_inode(ino_t id) {
	LOCK(_inode_cache_lock);
	for(size_t i = 0; i < _inode_cache.size(); i++) {
		if(_inode_cache[i]->id == id) return _inode_cache[i];
	}
	return -ENOENT;
}

void Filesystem::add_cached_inode(const DC::shared_ptr<Inode> &inode) {
	LOCK(_inode_cache_lock);
	_inode_cache.push_back(inode);
}

void Filesystem::remove_cached_inode(ino_t id) {
	LOCK(_inode_cache_lock);
	for(size_t i = 0; i < _inode_cache.size(); i++) {
		if(_inode_cache[i]->id == id) {
			_inode_cache.erase(i);
			return;
		}
	}
}