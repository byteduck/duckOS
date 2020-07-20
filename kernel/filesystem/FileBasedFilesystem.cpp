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

FileBasedFilesystem::FileBasedFilesystem(DC::shared_ptr<FileDescriptor> file): Filesystem(file) {

}

FileBasedFilesystem::~FileBasedFilesystem() {
	for(size_t i = 0; i < cache.size(); i++)
		if (cache[i].data) delete cache[i].data;
}

bool FileBasedFilesystem::read_logical_blocks(size_t block, size_t count, uint8_t *buffer) {
	if(_file->seek(block * logical_block_size(), SEEK_SET) < 0) return false;
	return _file->read(buffer, count * logical_block_size()) >= 0;
}

size_t FileBasedFilesystem::logical_block_size() {
	return _logical_block_size;
}

bool FileBasedFilesystem::read_block(size_t block, uint8_t *buffer) {
	LOCK(lock);
	BlockCacheEntry* cache_entry = get_chache_entry(block);
	if(cache_entry) {
		memcpy(buffer, cache_entry->data, block_size());
		return true;
	}

	if(_file->seek(block * block_size(), SEEK_SET) < 0) return false;
	cache_entry = make_cache_entry(block);
	if(_file->read(cache_entry->data, block_size()) > 0) {
		memcpy(buffer, cache_entry->data, block_size());
		return true;
	} else {
		free_cache_entry(block);
		return false;
	}
}

bool FileBasedFilesystem::read_blocks(size_t block, size_t count, uint8_t *buffer) {
	for(size_t i = 0; i < count; i++) {
		if(!read_block(block + i, buffer + block_size() * i)) return false;
	}
	return true;
}

bool FileBasedFilesystem::write_blocks(size_t block, size_t count, const uint8_t* buffer) {
	for(size_t i = 0; i < count; i++)
		if(!write_block(block + i, buffer + i * block_size())) return false;
	return true;
}

bool FileBasedFilesystem::write_block(size_t block, const uint8_t* buffer) {
	LOCK(lock);
	BlockCacheEntry* entry = get_chache_entry(block);
	if(!entry) {
		entry = make_cache_entry(block);
		if(_file->read(entry->data, block_size()) <= 0) return false;
	}
	memcpy(entry->data, buffer, block_size());
	entry->dirty = true;
	return true;
}

BlockCacheEntry* FileBasedFilesystem::get_chache_entry(size_t block) {
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
		if(entry->dirty) flush_cache_entry(entry, false);
		entry->block = block;
	} else {
		cache.push_back(BlockCacheEntry(block, new uint8_t[block_size()]));
		entry = &cache[cache.size() - 1];
	}
	entry->last_used = PIT::get_seconds();
	return entry;
}

void FileBasedFilesystem::flush_cache_entry(BlockCacheEntry* entry, bool use_lock) {
	if(use_lock) lock.acquire();
	ssize_t nwrote;
	if(_file->seek(entry->block * block_size(), SEEK_SET) < 0) return;
	if((nwrote =_file->write(entry->data, block_size())) == block_size()) {
		entry->dirty = false;
	} else {
		printf("WARNING: Error writing filesystem cache entry to disk! (Block %d, Error %d)\n", entry->block, nwrote);
	}
	if(use_lock) lock.release();
}

void FileBasedFilesystem::free_cache_entry(size_t block) {
	for(size_t i = 0; i < cache.size(); i++) {
		if (cache[i].block == block) {
			delete[] cache[i].data;
			cache.erase(i);
			return;
		}
	}
}

void FileBasedFilesystem::flush_cache() {
	for(size_t i = 0; i < cache.size(); i++) {
		if(cache[i].dirty) flush_cache_entry(&cache[i]);
	}
}
