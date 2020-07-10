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

#ifndef DUCKOS_MEMORYMAP_H
#define DUCKOS_MEMORYMAP_H


#include <common/cstddef.h>
#include "MemoryRegion.h"

class MemoryMap {
public:
	MemoryMap(size_t page_size, MemoryRegion* first_region);
	~MemoryMap();

	/**
	 * Allocates a memory region with a size of at least minimum_size.
	 * @param minimum_size The minimum size of the allocated region (will be rounded up to page boundary)
	 * @param storage If not null, the newly allocated region will be stored here (if a new one needs to be allocated)
	 * @return The region allocated. Will be nullptr if allocation failed.
	 */
	MemoryRegion* allocate_region(size_t minimum_size, MemoryRegion* storage = nullptr);

	/**
	 * Allocates a region that contains the address given and is at least minimum_size.
	 * @param address The address that the returned region should contain.
	 * @param minimum_size The minimum size of the region returned (will be rounded up to page boundary)
	 * @param storage If not null, the newly allocated region(s) will be stored here (if they need to be allocated)
	 * @return The memory region allocated. Will be nullptr if allocation failed.
	 */
	MemoryRegion* allocate_region(size_t address, size_t minimum_size, MemoryRegion storage[2] = nullptr);

	/**
	 * Frees the region given.
	 * If the regions adjacent to it are free, they will be merged and the adjacent region(s) will be deleted.
	 * @param region The region to free. Both its start and size may be mutated.
	 */
	void free_region(MemoryRegion* region);

	/**
	 * Finds the region containing the address.
	 * @param address The address to find the region of.
	 * @return The region corresponding to the address. nullptr if not found.
	 */
	MemoryRegion* find_region(size_t address);

	/**
	 * @return The amount of memory used in bytes. Doesn't count reserved memory.
	 */
	size_t used_memory();

	/**
	 * @return The amount of reserved memory in bytes.
	 */
	size_t reserved_memory();

	/**
	 * Recalculates the amount of used and reserved memory. Handy if entries were updated manually.
	 */
	void recalculate_memory_totals();

	/**
	 * @return The page size of the map.
	 */
	size_t page_size();

	MemoryRegion* first_region();

	void replace_entry(MemoryRegion *old_region, MemoryRegion *new_region);

private:
	MemoryRegion* _first_region;
	size_t _page_size;
	size_t bytes_used;
	size_t bytes_reserved;
};


#endif //DUCKOS_MEMORYMAP_H
