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

#include <common/defines.h>
#include <kernel/pit.h>
#include "FileBasedFilesystem.h"

BlockCacheEntry::BlockCacheEntry(size_t block, uint8_t *data): block(block), data(data) {

}

FileBasedFilesystem::FileBasedFilesystem(const DC::shared_ptr<FileDescriptor>& file): _file(file) {

}

FileBasedFilesystem::~FileBasedFilesystem() {
	for(size_t i = 0; i < cache.size(); i++)
		if (cache[i].data) delete cache[i].data;
}

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

	//If we have a cache entry already, just read that
	BlockCacheEntry *cache_entry = get_chache_entry(block);
	if (cache_entry) {
		memcpy(buffer, cache_entry->data, block_size());
		return SUCCESS;
	}

	//Try seeking the file
	int seekres = _file->seek(block * block_size(), SEEK_SET);
	if (seekres < 0) return seekres;

	//Make a cache entry
	cache_entry = make_cache_entry(block);

	//Read into the cache entry
	ssize_t nread = _file->read(cache_entry->data, block_size());
	if (nread <= 0) {
		//If we failed, free it and return the appropriate error
		free_cache_entry(block);
		if (nread == 0) return -EIO;
		return nread;
	}

	//If we succeeded, copy the cache data into the buffer and return success
	memcpy(buffer, cache_entry->data, block_size());
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
	BlockCacheEntry* entry = get_chache_entry(block);
	if(!entry) entry = make_cache_entry(block);
	memcpy(entry->data, buffer, block_size());
	entry->dirty = true;
	return SUCCESS;
}

Result FileBasedFilesystem::zero_block(size_t block) {
	LOCK(lock);
	BlockCacheEntry* entry = get_chache_entry(block);
	if(!entry) entry = make_cache_entry(block);
	memset(entry->data, 0, block_size());
	entry->dirty = true;
	return SUCCESS;
}

Result FileBasedFilesystem::truncate_block(size_t block, size_t new_size) {
	if(new_size >= block_size()) return -EOVERFLOW;
	LOCK(lock);
	BlockCacheEntry* entry = get_chache_entry(block);
	if(!entry) {
		entry = make_cache_entry(block);
		int res = _file->seek(block * block_size(), SEEK_SET);
		if(res < 0) return res;
		res = _file->read(entry->data, block_size());
		if(res < 0) return res;
		if(res != block_size()) return -EIO;
	}
	memset(entry->data + new_size, 0, (int)(block_size() - new_size));
	entry->dirty = true;
	return SUCCESS;
}

BlockCacheEntry* FileBasedFilesystem::get_chache_entry(size_t block) {
	LOCK(lock);
	for(size_t i = 0; i < cache.size(); i++) {
		if (cache[i].block == block) {
			BlockCacheEntry* entry = &cache[i];
			entry->last_used = PIT::get_seconds();
			return entry;
		}
	}
	return nullptr;
}

BlockCacheEntry* FileBasedFilesystem::make_cache_entry(size_t block) {
	LOCK(lock);
	BlockCacheEntry* entry;
	if(cache.size() >= MAX_FILESYSTEM_CACHE_SIZE / block_size()) {
		//If the cache is full, find the oldest entry and replace it
		size_t oldest_entry = 0;
		size_t oldest_entry_time = 0xFFFFFFFF;
		for(size_t i = 0; i < cache.size(); i++) {
			if(cache[i].last_used < oldest_entry_time) {
				oldest_entry = i;
				oldest_entry_time = cache[i].last_used;
			}
		}
		//Update entry's data
		entry = &cache[oldest_entry];
		if(entry->dirty) flush_cache_entry(entry);
		entry->block = block;
	} else {
		cache.push_back(BlockCacheEntry(block, new uint8_t[block_size()]));
		entry = &cache[cache.size() - 1];
	}
	entry->last_used = PIT::get_seconds();
	return entry;
}

void FileBasedFilesystem::flush_cache_entry(BlockCacheEntry* entry) {
	LOCK(lock);
	ssize_t nwrote;
	if(_file->seek(entry->block * block_size(), SEEK_SET) < 0) return;
	if((nwrote =_file->write(entry->data, block_size())) == block_size()) {
		entry->dirty = false;
	} else {
		printf("WARNING: Error writing filesystem cache entry to disk! (Block %d, Error %d)\n", entry->block, nwrote);
	}
}

void FileBasedFilesystem::free_cache_entry(size_t block) {
	LOCK(lock);
	for(size_t i = 0; i < cache.size(); i++) {
		if (cache[i].block == block) {
			delete[] cache[i].data;
			cache.erase(i);
			return;
		}
	}
}

ResultRet<DC::shared_ptr<Inode>> FileBasedFilesystem::get_cached_inode(ino_t id) {
	LOCK(_inode_cache_lock);
	for(size_t i = 0; i < _inode_cache.size(); i++) {
		if(_inode_cache[i]->id == id) return _inode_cache[i];
	}
	return -ENOENT;
}

void FileBasedFilesystem::add_cached_inode(const DC::shared_ptr<Inode> &inode) {
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

void FileBasedFilesystem::flush_cache() {
	LOCK(lock);
	for(size_t i = 0; i < cache.size(); i++) {
		if(cache[i].dirty) flush_cache_entry(&cache[i]);
	}
}

Inode* FileBasedFilesystem::get_inode_rawptr(ino_t id) {
	return nullptr;
}

ResultRet<DC::shared_ptr<Inode>> FileBasedFilesystem::get_inode(ino_t id) {
	LOCK(_inode_cache_lock);
	auto inode_perhaps = get_cached_inode(id);
	if(inode_perhaps.is_error()) {
		Inode* in = get_inode_rawptr(id);
		if(in) {
			auto ins = DC::shared_ptr<Inode>(in);
			add_cached_inode(ins);
			return ins;
		} else return -ENOENT;
	} else {
		return inode_perhaps.value();
	}
}
