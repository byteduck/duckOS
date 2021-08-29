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

#include "MemoryRegion.h"
#include "Memory.h"
#include "MemoryMap.h"
#include <kernel/kstd/vector.hpp>
#include <kernel/kstd/kstdio.h>

MemoryRegion::MemoryRegion(size_t start, size_t size): start(start), size(size), next(nullptr), prev(nullptr), heap_allocated(true) {

}

MemoryRegion::MemoryRegion(const MemoryRegion& region):
	start(region.start),
	size(region.size),
	related(region.related),
	used(region.used),
	heap_allocated(true),
	reserved(region.reserved),
	cow(region.cow),
	is_shm(region.is_shm),
	shm_refs(region.shm_refs),
	shm_id(region.shm_id),
	shm_owner(region.shm_owner)
{
	if(region.shm_allowed) {
		shm_allowed = new kstd::vector<ShmPermissions>(*region.shm_allowed);
	}
}

MemoryRegion::~MemoryRegion() {
	delete shm_allowed;
}

void MemoryRegion::free() {
	delete shm_allowed;
	related = nullptr;
	used = false;
	reserved = false;
	cow.num_refs = 0;
	is_shm = false;
	shm_refs = 0;
	shm_id = 0;
	shm_owner = -1;
	shm_allowed = nullptr;
}

void MemoryRegion::cow_deref() {
	lock.acquire();
	cow.num_refs--;
	if(!cow.num_refs) {
		Memory::pmem_map().free_region(this);
	}
	lock.release();
}

void MemoryRegion::shm_ref() {
	lock.acquire();
	shm_refs++;
	lock.release();
}

void MemoryRegion::shm_deref() {
	lock.acquire();
	shm_refs--;
	if(!shm_refs) {
		lock.release();
		Memory::pmem_map().free_region(this);
		return;
	}
	lock.release();
}

size_t MemoryRegion::end() {
	return start + size - 1;
}

void MemoryRegion::print(bool print_related) {
	printf("{%x -> %x}(%s%s", start, end(), used ? "Used" : "Free", reserved ? ", Reserved" : "");
	if(cow.marked_cow)
		printf(", CoW[%d]", cow.num_refs);
	if(is_shm)
		printf(", Shared[%d]", shm_owner);
	printf(")");
	if(print_related && related) {
		printf(" => ");
		related->print(false);
	} else
		printf("\n");
}