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

#ifndef DUCKOS_FILEBASEDFILESYSTEM_H
#define DUCKOS_FILEBASEDFILESYSTEM_H

#include "FileSystem.h"

#define MAX_FILESYSTEM_CACHE_SIZE 0x1000000 //16 MiB

class BlockCacheEntry {
public:
	BlockCacheEntry(size_t block, uint8_t* data);
	size_t block = 0;
	uint8_t* data = nullptr;
	size_t last_used = 0;
	bool dirty = false;
};

class FileBasedFilesystem: public Filesystem {
public:
	FileBasedFilesystem(DC::shared_ptr<FileDescriptor> file);
	~FileBasedFilesystem();
	size_t logical_block_size();
	bool read_logical_blocks(size_t block, size_t count, uint8_t* buffer);
	bool read_block(size_t block, uint8_t* buffer);
	bool read_blocks(size_t block, size_t count, uint8_t* buffer);
protected:
	BlockCacheEntry* get_chache_entry(size_t block);
	BlockCacheEntry* make_cache_entry(size_t block);
	void flush_cache_entry(BlockCacheEntry* entry);
	void free_cache_entry(size_t block);

	size_t _logical_block_size {512};
	DC::vector<BlockCacheEntry> cache;
};


#endif //DUCKOS_FILEBASEDFILESYSTEM_H
