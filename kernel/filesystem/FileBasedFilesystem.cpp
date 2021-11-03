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

#include "FileBasedFilesystem.h"
#include <kernel/kstd/cstring.h>
#include <kernel/time/Time.h>
#include "Inode.h"
#include "FileDescriptor.h"

FileBasedFilesystem::FileBasedFilesystem(const kstd::shared_ptr<FileDescriptor>& file): _file(file) {

}

FileBasedFilesystem::~FileBasedFilesystem() = default;

Result FileBasedFilesystem::read_logical_block(size_t block, uint8_t *buffer) {
	return read_logical_blocks(block, 1, buffer);
}

Result FileBasedFilesystem::read_logical_blocks(size_t block, size_t count, uint8_t *buffer) {
	LOCK(lock);
	int code = _file->seek(block * logical_block_size(), SEEK_SET);
	if(code < 0) return code;

	ssize_t nread = _file->read(buffer, count * logical_block_size());
	if(nread < 0) return nread;
	if(nread != count * logical_block_size()) return -EIO;
	return SUCCESS;
}

Result FileBasedFilesystem::write_logical_block(size_t block, const uint8_t *buffer) {
	return write_logical_blocks(block, 1, buffer);
}

Result FileBasedFilesystem::write_logical_blocks(size_t block, size_t count, const uint8_t *buffer) {
	LOCK(lock);
	int code = _file->seek(block * logical_block_size(), SEEK_SET);
	if(code < 0) return code;

	ssize_t nwrote = _file->write(buffer, count * logical_block_size());
	if(nwrote < 0) return nwrote;
	if(nwrote != count * logical_block_size()) return -EIO;
	return SUCCESS;
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
	LOCK(lock);

	int res = _file->seek(block * block_size(), SEEK_SET);
	if (res < 0)
		return res;

	ssize_t nread = _file->read(buffer, block_size());
	if (nread == 0)
		return -EIO;
	else if(nread < 0)
		return nread;

	return SUCCESS;
}

Result FileBasedFilesystem::read_blocks(size_t block, size_t count, uint8_t *buffer) {
	Result res = SUCCESS;
	for(size_t i = 0; i < count; i++) {
		res = read_block(block + i, buffer + block_size() * i);
		if(res.is_error()) return res;
	}
	return SUCCESS;
}

Result FileBasedFilesystem::write_blocks(size_t block, size_t count, const uint8_t* buffer) {
	Result res = SUCCESS;
	for(size_t i = 0; i < count; i++) {
		res = write_block(block + i, buffer + i * block_size());
		if(res.is_error()) return res;
	}
	return SUCCESS;
}

Result FileBasedFilesystem::write_block(size_t block, const uint8_t* buffer) {
	LOCK(lock);

	int res = _file->seek(block * block_size(), SEEK_SET);
	if(res < 0)
		return res;

	ssize_t nwrote = _file->write(buffer, block_size());
	if(nwrote == 0)
		return -EIO;
	else if(nwrote < 0)
		return nwrote;

	return SUCCESS;
}

Result FileBasedFilesystem::zero_block(size_t block) {
	LOCK(lock);

	//TODO: Implementation that doesn't require allocating a buffer

	int res = _file->seek(block * block_size(), SEEK_SET);
	if(res < 0)
		return res;

	auto* zero_buf = new uint8_t[block_size()];
	memset(zero_buf, 0, block_size());

	ssize_t nwrote = _file->write(zero_buf, block_size());
	delete[] zero_buf;

	if(nwrote == 0)
		return -EIO;
	else if(nwrote < 0)
		return nwrote;

	return SUCCESS;
}

Result FileBasedFilesystem::truncate_block(size_t block, size_t new_size) {
	if(new_size >= block_size()) return -EOVERFLOW;
	LOCK(lock);

	//TODO: Implementation that doesn't require allocating a buffer

	int res = _file->seek(block * block_size(), SEEK_SET);
	if(res < 0)
		return res;

	auto* buf = new uint8_t[block_size()];
	res = _file->read(buf, block_size());
	if(res < 0) {
		delete[] buf;
		return res;
	}

	memset(buf + new_size, 0, (int)(block_size() - new_size));
	ssize_t nwrote = _file->write(buf, block_size());
	delete[] buf;

	if(nwrote == 0)
		return -EIO;
	else if(nwrote < 0)
		return nwrote;

	return SUCCESS;
}

ResultRet<kstd::shared_ptr<Inode>> FileBasedFilesystem::get_cached_inode(ino_t id) {
	LOCK(_inode_cache_lock);
	for(size_t i = 0; i < _inode_cache.size(); i++) {
		if(_inode_cache[i]->id == id) return _inode_cache[i];
	}
	return -ENOENT;
}

void FileBasedFilesystem::add_cached_inode(const kstd::shared_ptr<Inode> &inode) {
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

ResultRet<kstd::shared_ptr<Inode>> FileBasedFilesystem::get_inode(ino_t id) {
	LOCK(_inode_cache_lock);
	auto inode_perhaps = get_cached_inode(id);
	if(inode_perhaps.is_error()) {
		Inode* in = get_inode_rawptr(id);
		if(in) {
			auto ins = kstd::shared_ptr<Inode>(in);
			add_cached_inode(ins);
			return ins;
		} else return -ENOENT;
	} else {
		return inode_perhaps.value();
	}
}
