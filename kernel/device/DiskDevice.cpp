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

#include <kernel/kstd/cstring.h>
#include <kernel/memory/MemoryManager.h>
#include "DiskDevice.h"
#include "kernel/kstd/KLog.h"

size_t DiskDevice::s_used_cache_memory = 0;
kstd::vector<DiskDevice*> DiskDevice::s_disk_devices;
Mutex DiskDevice::s_disk_devices_lock("DiskDevices");
BooleanBlocker DiskDevice::s_writeback_blocker;

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
	kstd::Arc<BlockCacheRegion> cache_region;
	for(size_t i = 0; i < count; i++) {
		size_t block = start_block + i;
		if(!cache_region || !cache_region->has_block(block))
			cache_region = get_cache_region(block);
		LOCK(cache_region->lock);
		cache_region->last_used = Time::now();
		memcpy(buffer + i * block_size(), cache_region->block_data(block), block_size());
	}
	return Result(SUCCESS);
}

Result DiskDevice::write_blocks(uint32_t start_block, uint32_t count, const uint8_t* buffer) {
	auto invalidate_cache_region = [this] (const kstd::Arc<BlockCacheRegion>& region) {
		LOCK(_dirty_regions_lock);
		if (_dirty_regions.contains(region->start_block))
			return;
		_dirty_regions.push_back(region->start_block);
	};

	kstd::Arc<BlockCacheRegion> cache_region;
	for(size_t i = 0; i < count; i++) {
		size_t block = start_block + i;
		if(!cache_region || !cache_region->has_block(block)) {
			if (cache_region)
				invalidate_cache_region(cache_region);
			cache_region = get_cache_region(block);
		}
		LOCK(cache_region->lock);
		cache_region->last_used = Time::now();
		cache_region->dirty = true;
		memcpy(cache_region->block_data(block), buffer + i * block_size(), block_size());
	}

	invalidate_cache_region(cache_region);
	s_writeback_blocker.set_ready(true);

	return Result::Success;
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
			if(device->_cache_regions.empty())
				continue;
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
		KLog::warn("DiskDevice", "Was asked to free {} pages, could only free {}...", num_pages, num_freed);

	return num_freed;
}

kstd::Arc<DiskDevice::BlockCacheRegion> DiskDevice::get_cache_region(size_t block) {
	_cache_lock.acquire();

	//See if we already have the block
	auto reg_opt = _cache_regions.get(block_cache_region_start(block));
	if(reg_opt) {
		_cache_lock.release();
		return reg_opt.value();
	}

	//Create a new cache region
	auto reg = kstd::Arc<BlockCacheRegion>::make(block_cache_region_start(block), block_size());
	s_used_cache_memory += PAGE_SIZE;
	reg->lock.acquire();
	_cache_regions.insert(block_cache_region_start(block), reg);

	//Read the blocks into it
	read_uncached_blocks(reg->start_block, blocks_per_cache_region(), (uint8_t*) reg->region->start());

	//TODO: Figure out how to read the block after releasing the cache lock so that other blocks can be used in the meantime
	//(We cannot do this currently as that would result in acquiring / releasing locks in the wrong order)
	reg->lock.release();
	_cache_lock.release();

	//Return the requested region
	return reg;
}

void DiskDevice::cache_writeback_task_entry() {
	s_writeback_blocker.set_ready(false);
	static constexpr bool writeback_debug = false;
	while (true) {
		TaskManager::current_thread()->block(s_writeback_blocker);
		s_writeback_blocker.set_ready(false);

		KLog::dbg_if<writeback_debug>("DiskDevice", "Writing back caches...");
		for (auto device : s_disk_devices) {
			while (true) {
				size_t region_loc;
				{
					LOCK(device->_dirty_regions_lock);
					if (device->_dirty_regions.empty())
						break;
					region_loc = device->_dirty_regions.pop_front();
				}
				kstd::Arc<BlockCacheRegion> region;
				{
					LOCK(device->_cache_lock);
					auto region_opt = device->_cache_regions.get(region_loc);
					if (!region_opt) {
						KLog::warn("DiskDevice", "Was going to write back cache region, but couldn't find it!");
						return;
					}
					region = region_opt.value();
				}
				LOCK(region->lock);
				device->write_uncached_blocks(region->start_block, region->num_blocks(), (uint8_t*) region->region->start());
				region->dirty = false;
			}
		}
		KLog::dbg_if<writeback_debug>("DiskDevice", "Done writing caches!");
	}
}

DiskDevice::BlockCacheRegion::BlockCacheRegion(size_t start_block, size_t block_size):
		region(MemoryManager::inst().alloc_kernel_region(PAGE_SIZE)), block_size(block_size), start_block(start_block) {}

DiskDevice::BlockCacheRegion::~BlockCacheRegion() = default;
