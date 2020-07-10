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

#ifndef DUCKOS_MEMORYREGION_H
#define DUCKOS_MEMORYREGION_H


#include <common/cstddef.h>

class MemoryRegion {
public:
	MemoryRegion() = default;
	MemoryRegion(size_t start, size_t size);
	MemoryRegion(const MemoryRegion& region);
	~MemoryRegion();

	//Used to decrease the number of CoW references on a physical memory region & free the region if necessary.
	void cow_deref();

	//The memory location start of the region.
	size_t start;

	//The size, in bytes, of the region.
	size_t size;

	//The next region in the linked list of regions.
	MemoryRegion* next;

	//The previous region in the linked list of regions.
	MemoryRegion* prev;

	//The region related to this one (e.g. the physical region corresponding to a virtual region or vice versa)
	MemoryRegion* related = nullptr;

	//Whether or not the region is used.
	bool used = false;

	//Whether or not the region was allocated on the heap (if true, it will be deleted by MemoryMap when removed)
	bool heap_allocated = true;

	//Whether or not the region is reserved. (e.g. memory-mapped hardware)
	bool reserved = false;

	//If this is a virtual region, marked_cow is used to determine if a region is marked CoW.
	//If this is a physical region, num_refs is used to determine the number of times the physical region is referenced.
	union cow {
		bool marked_cow;
		size_t num_refs = 0;
	} cow;
};


#endif //DUCKOS_MEMORYREGION_H
