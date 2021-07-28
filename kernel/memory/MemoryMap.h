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


#include <kernel/kstd/types.h>
#include <kernel/tasking/SpinLock.h>

class MemoryRegion;
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
	 * Allocates a memory region with a size of at least minimum_size, starting the search from the end of the region and working backwards.
	 * @param minimum_size The minimum size of the allocated region (will be rounded up to page boundary)
	 * @param storage If not null, the newly allocated region will be stored here (if a new one needs to be allocated)
	 * @return The region allocated. Will be nullptr if allocation failed.
	 */
	MemoryRegion* allocate_stack_region(size_t minimum_size, MemoryRegion* storage = nullptr);

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
	 * Splits the region given into multiple parts.
	 * @param start The start of the region to be returned.
	 * @param size The size of the region to be returned.
	 * @return The memory region given by the range specified, or null if the range was invalid.
	 */
	MemoryRegion* split_region(MemoryRegion* region, size_t start, size_t size);

	/**
	 * Finds the region containing the address.
	 * @param address The address to find the region of.
	 * @return The region corresponding to the address. nullptr if not found.
	 */
	MemoryRegion* find_region(size_t address);

	/**
	 * Finds the shared region corresponding to the id.
	 * @param id The id of the shared region.
	 * @return The region corresponding to the id. nullptr if not found.
	 */
	MemoryRegion* find_shared_region(int id);

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

	SpinLock lock;
private:
	size_t _page_size = 0;
	MemoryRegion* _first_region = nullptr;
	MemoryRegion* _last_region = nullptr;
	size_t bytes_used = 0;
	size_t bytes_reserved = 0;
};


#endif //DUCKOS_MEMORYMAP_H
