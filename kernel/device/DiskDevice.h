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

#pragma once

#include <kernel/time/Time.h>
#include <kernel/memory/LinkedMemoryRegion.h>
#include <kernel/memory/MemoryManager.h>
#include "BlockDevice.h"
#include <kernel/kstd/map.hpp>

class DiskDevice: public BlockDevice {
public:
	DiskDevice(unsigned major, unsigned minor): BlockDevice(major, minor) {}

	Result read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) override final;
	Result write_blocks(uint32_t block, uint32_t count, const uint8_t *buffer) override final;

	virtual Result read_uncached_blocks(uint32_t block, uint32_t count, uint8_t *buffer) = 0;
	virtual Result write_uncached_blocks(uint32_t block, uint32_t count, const uint8_t *buffer) = 0;

	static size_t used_cache_memory();

private:
	class BlockCacheRegion {
	public:
		explicit BlockCacheRegion(size_t start_block, size_t block_size);
		~BlockCacheRegion();

		inline bool has_block(size_t block) const { return block >= start_block && block < start_block + num_blocks(); }
		inline size_t num_blocks() const { return PAGE_SIZE / block_size; }
		inline uint8_t* block_data(size_t block) const { return (uint8_t*) (region.virt->start + block_size * (block - start_block)); }

		LinkedMemoryRegion region;
		size_t block_size;
		size_t start_block;
		Time last_used = Time::now();
		bool dirty = false;
	};

	BlockCacheRegion* get_cache_region(size_t block);
	inline size_t blocks_per_cache_region() { return PAGE_SIZE / block_size(); }
	inline size_t block_cache_region_start(size_t block) { return block - (block % blocks_per_cache_region()); }

	//TODO: Free cache regions when low on memory
	kstd::map<size_t, kstd::shared_ptr<BlockCacheRegion>> _cache_regions;
	SpinLock _cache_lock;

	static size_t _used_cache_memory;
};

