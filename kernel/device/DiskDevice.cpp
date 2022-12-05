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
#include <kernel/kstd/cstring.h>
#include <kernel/memory/MemoryManager.h>
#include "DiskDevice.h"
#include "kernel/kstd/KLog.h"

size_t DiskDevice::s_used_cache_memory = 0;
kstd::vector<DiskDevice*> DiskDevice::s_disk_devices;
SpinLock DiskDevice::s_disk_devices_lock;

DiskDevice::DiskDevice(unsigned int major, unsigned int minor): BlockDevice(major, minor) {
	s_disk_devices.push_back(this);
}

DiskDevice::~DiskDevice() {
	LOCK(s_disk_devices_lock);
	for(size_t i = 0; i < s_disk_devices.size(); i++) {
		if(s_disk_devices[i] == this) {
			s_disk_devices.erase(i);
			break;
		}
	}
};

Result DiskDevice::read_blocks(uint32_t start_block, uint32_t count, uint8_t* buffer) {
	LOCK(_cache_lock);
	kstd::Arc<BlockCacheRegion> cache_region;
	for(size_t i = 0; i < count; i++) {
		size_t block = start_block + i;
		if(!cache_region || !cache_region->has_block(block))
			cache_region = get_cache_region(block);
		cache_region->last_used = Time::now();
		memcpy(buffer + i * block_size(), cache_region->block_data(block), block_size());
	}
	return Result(SUCCESS);
}

Result DiskDevice::write_blocks(uint32_t start_block, uint32_t count, const uint8_t* buffer) {
	LOCK(_cache_lock);
	kstd::Arc<BlockCacheRegion> cache_region;
	for(size_t i = 0; i < count; i++) {
		size_t block = start_block + i;
		if(!cache_region || !cache_region->has_block(block))
			cache_region = get_cache_region(block);
		cache_region->last_used = Time::now();
		cache_region->dirty = true;
		memcpy(cache_region->block_data(block), buffer + i * block_size(), block_size());
	}

	//TODO: Flush cached writes to disk periodically instead of on every write
	return write_uncached_blocks(start_block, count, buffer);
}

size_t DiskDevice::used_cache_memory() {
	return s_used_cache_memory;
}

size_t DiskDevice::free_pages(size_t num_pages) {
	size_t num_freed = 0;
	LOCK(s_disk_devices_lock);

	while(num_freed < num_pages) {
		DiskDevice* lru_device = nullptr;
		Time lru_time = Time::distant_future();
		kstd::Arc<BlockCacheRegion> lru_region;

		// Find the device with the least recently used cache region
		for(size_t i = 0; i < s_disk_devices.size(); i++) {
			auto device = s_disk_devices[i];
			LOCK_N(device->_cache_lock, device_lock);
			auto device_lru = device->_cache_regions.lru_unsafe();
			auto time = device_lru.second->last_used;
			if(time < lru_time) {
				lru_time = time;
				lru_device = device;
				lru_region = device_lru.second;
			}
		}

		if(!lru_region)
			break;

		// Flush it if necessary
		if(lru_region->dirty)
			lru_device->write_uncached_blocks(lru_region->start_block, lru_region->num_blocks(), (uint8_t*) lru_region->region->start());

		// Free it
		num_freed += lru_region->region->size() / PAGE_SIZE;
		s_used_cache_memory -= lru_region->region->size();
		lru_region.reset();
		lru_device->_cache_regions.prune(1);
	}

	if(num_freed != num_pages)
		KLog::warn("DiskDevice", "Was asked to free %d pages, could only free %d...", num_pages, num_freed);

	return num_freed;
}

kstd::Arc<DiskDevice::BlockCacheRegion> DiskDevice::get_cache_region(size_t block) {
	LOCK(_cache_lock);

	//See if we already have the block
	auto reg_opt = _cache_regions.get(block_cache_region_start(block));
	if(reg_opt)
		return reg_opt.move_value();

	//Create a new cache region
	auto reg = kstd::Arc<BlockCacheRegion>::make(block_cache_region_start(block), block_size());
	s_used_cache_memory += PAGE_SIZE;

	//Read the blocks into it
	read_uncached_blocks(reg->start_block, blocks_per_cache_region(), (uint8_t*) reg->region->start());

	//Return the requested region
	_cache_regions.insert(block_cache_region_start(block), reg);
	return reg;
}

DiskDevice::BlockCacheRegion::BlockCacheRegion(size_t start_block, size_t block_size):
		region(MemoryManager::inst().alloc_kernel_region(PAGE_SIZE)), block_size(block_size), start_block(start_block) {}

DiskDevice::BlockCacheRegion::~BlockCacheRegion() = default;
