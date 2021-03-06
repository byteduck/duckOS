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

#include <kernel/kstdio.h>
#include "MemoryRegion.h"
#include "Memory.h"
#include <common/vector.hpp>

MemoryRegion::MemoryRegion(size_t start, size_t size): start(start), size(size), next(nullptr), prev(nullptr), heap_allocated(true) {

}

MemoryRegion::MemoryRegion(const MemoryRegion& region): start(region.start), size(region.size), next(region.next), prev(region.prev), heap_allocated(true) {

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
