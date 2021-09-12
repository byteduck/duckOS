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

#include <kernel/memory/PageDirectory.h>
#include <cstring>
#include <kernel/memory/MemoryManager.h>
#include "DiskDevice.h"

size_t DiskDevice::_used_cache_memory = 0;

Result DiskDevice::read_blocks(uint32_t start_block, uint32_t count, uint8_t* buffer) {
	LOCK(_cache_lock);
	for(size_t i = 0; i < count; i++) {
		size_t block = start_block + i;
		BlockCacheEntry* entry = get_cache_entry(block);
		entry->last_used = Time::now();
		memcpy(buffer + i * block_size(), entry->data, block_size());
	}
	return SUCCESS;
}

Result DiskDevice::write_blocks(uint32_t start_block, uint32_t count, const uint8_t* buffer) {
	LOCK(_cache_lock);
	for(size_t i = 0; i < count; i++) {
		size_t block = start_block + i;
		BlockCacheEntry* entry = get_cache_entry(block);
		entry->last_used = Time::now();
		entry->dirty = true;
		memcpy(entry->data, buffer + i * block_size(), block_size());
	}

	//TODO: Flush cached writes to disk periodically instead of on every write
	return write_uncached_blocks(start_block, count, buffer);
}

DiskDevice::~DiskDevice() {
	for(auto i = 0; i < _cache_regions.size(); i++)
		delete _cache_regions[i];
}

size_t DiskDevice::used_cache_memory() {
	return _used_cache_memory;
}

DiskDevice::BlockCacheEntry* DiskDevice::get_cache_entry(size_t block) {
	//See if any cache regions already have the block we're looking for
	for(auto i = 0; i < _cache_regions.size(); i++) {
		auto block_res = _cache_regions[i]->get_cached_block(block);
		if(!block_res.is_error())
			return block_res.value();
	}

	//Create a new cache region
	auto* reg = new BlockCacheRegion(block_cache_region_start(block), block_size());
	_cache_regions.push_back(reg);
	_used_cache_memory += PAGE_SIZE;

	//Read the blocks into it
	read_uncached_blocks(reg->start_block, blocks_per_cache_region(), (uint8_t*) reg->region.virt->start);

	//Return the requested block
	auto entry = reg->get_cached_block(block);
	ASSERT(!entry.is_error());
	return entry.value();
}

DiskDevice::BlockCacheEntry::BlockCacheEntry(size_t block, size_t size, uint8_t* data):
		block(block), size(size), data(data){}

DiskDevice::BlockCacheRegion::BlockCacheRegion(size_t start_block, size_t block_size):
		region(PageDirectory::k_alloc_region(PAGE_SIZE)), block_size(block_size), start_block(start_block)
{
	cached_blocks.resize(PAGE_SIZE / block_size);
	for(auto i = 0; i < num_blocks(); i++)
		cached_blocks[i] = BlockCacheEntry(start_block + i, block_size, block_data(i));
}

DiskDevice::BlockCacheRegion::~BlockCacheRegion() {
	PageDirectory::k_free_region(region);
}

ResultRet<DiskDevice::BlockCacheEntry*> DiskDevice::BlockCacheRegion::get_cached_block(size_t block) {
	if(block < start_block || block >= start_block + num_blocks())
		return -EINVAL;
	return &cached_blocks[block - start_block];
}
