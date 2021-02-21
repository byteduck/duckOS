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
#include <kernel/tasking/SpinLock.h>

namespace DC {
	template<typename T> class vector;
};

class MemoryRegion {
public:
	struct ShmPermissions {
		pid_t pid;
		bool write;
	};

	MemoryRegion() = default;
	MemoryRegion(size_t start, size_t size);
	MemoryRegion(const MemoryRegion& region);
	~MemoryRegion();

	//Used to reset all of the region's properties to defaults for a free region
	void free();

	//Used to decrease the number of CoW references on a physical memory region & free the region if necessary.
	void cow_deref();

	//Used to increase the number of shared memory references on a physical region.
	void shm_ref();

	//Used to decrease the number of shared memory references on a physical region & free it if necessary.
	void shm_deref();

	//The memory location start of the region.
	size_t start = 0;

	//The size, in bytes, of the region.
	size_t size = 0;

	//The next region in the linked list of regions.
	MemoryRegion* next = nullptr;

	//The previous region in the linked list of regions.
	MemoryRegion* prev = nullptr;

	//The region related to this one (e.g. the physical region corresponding to a virtual region or vice versa)
	//Invalid for physical shared memory and physical CoW regions.
	MemoryRegion* related = nullptr;

	//This lock will be used for various operations on the region.
	SpinLock lock;

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

	//Whether or not this region is for shared memory.
	bool is_shm = false;

	//If this is a physical shared region, this will count the number of processes using it.
	int shm_refs = 0;

	//If applicable, this will be set to the ID of the shared region.
	int shm_id = 0;

	//If applicable, the PID of the process that created the shared region. If the process dies, it will be set to -1.
	pid_t shm_owner = -1;

	//If applicable, this will be a list of processes with access to the shared region. Otherwise, it will be nullptr.
	DC::vector<ShmPermissions>* shm_allowed = nullptr;
};


#endif //DUCKOS_MEMORYREGION_H
