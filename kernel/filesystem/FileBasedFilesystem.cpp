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

#include "FileBasedFilesystem.h"
#include <kernel/kstd/cstring.h>
#include <kernel/time/Time.h>
#include "Inode.h"
#include "FileDescriptor.h"

FileBasedFilesystem::FileBasedFilesystem(const kstd::Arc<FileDescriptor>& file): _file(file) {

}

FileBasedFilesystem::~FileBasedFilesystem() = default;

Result FileBasedFilesystem::read_logical_block(size_t block, uint8_t *buffer) {
	return read_logical_blocks(block, 1, buffer);
}

Result FileBasedFilesystem::read_logical_blocks(size_t block, size_t count, uint8_t *buffer) {
	ssize_t nread = _file->file()->read(*_file, block * logical_block_size(), KernelPointer<uint8_t>(buffer), count * logical_block_size());
	if(nread < 0) return Result(nread);
	if(nread != count * logical_block_size()) return Result(-EIO);
	return Result(SUCCESS);
}

Result FileBasedFilesystem::write_logical_block(size_t block, const uint8_t *buffer) {
	return write_logical_blocks(block, 1, buffer);
}

Result FileBasedFilesystem::write_logical_blocks(size_t block, size_t count, const uint8_t *buffer) {
	ssize_t nwrote = _file->file()->write(*_file, block * logical_block_size(), KernelPointer<const uint8_t>(buffer), count * logical_block_size());
	if(nwrote < 0) return Result(nwrote);
	if(nwrote != count * logical_block_size()) return Result(-EIO);
	return Result(SUCCESS);
}

size_t FileBasedFilesystem::logical_block_size() {
	return _logical_block_size;
}

size_t FileBasedFilesystem::block_size() {
	return _block_size;
}

void FileBasedFilesystem::set_block_size(size_t block_size) {
	_block_size = block_size;
}

Result FileBasedFilesystem::read_block(size_t block, uint8_t *buffer) {
	ssize_t nread = _file->file()->read(*_file, block * block_size(), KernelPointer<uint8_t>(buffer), block_size());
	if (nread == 0)
		return Result(-EIO);
	else if(nread < 0)
		return Result(nread);

	return Result(SUCCESS);
}

Result FileBasedFilesystem::read_blocks(size_t block, size_t count, uint8_t *buffer) {
	Result res = Result(SUCCESS);
	for(size_t i = 0; i < count; i++) {
		res = read_block(block + i, buffer + block_size() * i);
		if(res.is_error())
			return res;
	}
	return Result(SUCCESS);
}

Result FileBasedFilesystem::write_blocks(size_t block, size_t count, const uint8_t* buffer) {
	Result res = Result(SUCCESS);
	for(size_t i = 0; i < count; i++) {
		res = write_block(block + i, buffer + i * block_size());
		if(res.is_error()) return Result(res);
	}
	return Result(SUCCESS);
}

Result FileBasedFilesystem::write_block(size_t block, const uint8_t* buffer) {
	ssize_t nwrote = _file->file()->write(*_file, block * block_size(), KernelPointer<const uint8_t>(buffer), block_size());
	if(nwrote == 0)
		return Result(-EIO);
	else if(nwrote < 0)
		return Result(nwrote);

	return Result(SUCCESS);
}

Result FileBasedFilesystem::zero_block(size_t block) {
	//TODO: Implementation that doesn't require allocating a buffer
	auto* zero_buf = new uint8_t[block_size()];
	memset(zero_buf, 0, block_size());

	ssize_t nwrote = _file->file()->write(*_file, block * block_size(), KernelPointer<const uint8_t>(zero_buf), block_size());
	delete[] zero_buf;

	if(nwrote == 0)
		return Result(-EIO);
	else if(nwrote < 0)
		return Result(nwrote);

	return Result(SUCCESS);
}

Result FileBasedFilesystem::truncate_block(size_t block, size_t new_size) {
	if(new_size >= block_size()) return Result(-EOVERFLOW);

	//TODO: Implementation that doesn't require allocating a buffer
	auto* buf = new uint8_t[block_size()];
	int res = _file->file()->read(*_file, block * block_size(), KernelPointer<uint8_t>(buf), block_size());
	if(res < 0) {
		delete[] buf;
		return Result(res);
	}

	memset(buf + new_size, 0, (int)(block_size() - new_size));
	ssize_t nwrote = _file->file()->write(*_file, block * block_size(), KernelPointer<uint8_t>(buf), block_size());
	delete[] buf;

	if(nwrote == 0)
		return Result(-EIO);
	else if(nwrote < 0)
		return Result(nwrote);

	return Result(SUCCESS);
}

ResultRet<kstd::Arc<Inode>> FileBasedFilesystem::get_cached_inode(ino_t id) {
	LOCK(_inode_cache_lock);
	for(size_t i = 0; i < _inode_cache.size(); i++) {
		if(_inode_cache[i]->id == id) return _inode_cache[i];
	}
	return Result(-ENOENT);
}

void FileBasedFilesystem::add_cached_inode(const kstd::Arc<Inode> &inode) {
	LOCK(_inode_cache_lock);
	_inode_cache.push_back(inode);
}

void FileBasedFilesystem::remove_cached_inode(ino_t id) {
	LOCK(_inode_cache_lock);
	for(size_t i = 0; i < _inode_cache.size(); i++) {
		if(_inode_cache[i]->id == id) {
			_inode_cache.erase(i);
			return;
		}
	}
}

Inode* FileBasedFilesystem::get_inode_rawptr(ino_t id) {
	return nullptr;
}

ResultRet<kstd::Arc<Inode>> FileBasedFilesystem::get_inode(ino_t id) {
	LOCK(_inode_cache_lock);
	auto inode_perhaps = get_cached_inode(id);
	if(inode_perhaps.is_error()) {
		Inode* in = get_inode_rawptr(id);
		if(in) {
			auto ins = kstd::Arc<Inode>(in);
			add_cached_inode(ins);
			return ins;
		} else return Result(-ENOENT);
	} else {
		return inode_perhaps.value();
	}
}
