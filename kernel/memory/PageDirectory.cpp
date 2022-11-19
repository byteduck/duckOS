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

#include <kernel/kstd/vector.hpp>
#include <kernel/tasking/TaskManager.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/kstd/defines.h>
#include <kernel/Atomic.h>
#include "PageTable.h"
#include "MemoryRegion.h"
#include "LinkedMemoryRegion.h"
#include "MemoryManager.h"
#include <kernel/kstd/cstring.h>

/*
 * These variables are stored in char arrays in order to avoid re-initializing them when we call global constructors,
 * as they will have already been initialized by init_kmem().
 */
__attribute__((aligned(4096))) uint8_t __kernel_page_table_entries_storage[sizeof(PageTable::Entry) * 256 * 1024];
uint8_t __kernel_entries_storage[sizeof(PageDirectory::Entry) * 256];
uint8_t __kernel_page_tables_storage[sizeof(PageTable) * 256];
uint8_t __kernel_vmem_map_storage[sizeof(MemoryMap)];
uint8_t __early_vmem_regions_storage[sizeof(MemoryRegion) * 3];

PageDirectory::Entry (&PageDirectory::kernel_entries)[256] = (PageDirectory::Entry(&)[256]) *__kernel_entries_storage;
PageTable (&PageDirectory::kernel_page_tables)[256] = (PageTable(&)[256]) *__kernel_page_tables_storage;
size_t PageDirectory::kernel_page_tables_physaddr[1024];
MemoryMap& PageDirectory::kernel_vmem_map = (MemoryMap&) *__kernel_vmem_map_storage;
MemoryRegion (&PageDirectory::early_vmem_regions)[3] = (MemoryRegion(&)[3]) *__early_vmem_regions_storage;
PageTable::Entry (&kernel_page_table_entries)[256][1024] = (PageTable::Entry(&)[256][1024]) *__kernel_page_table_entries_storage;
size_t PageDirectory::used_kernel_pmem;
size_t PageDirectory::used_kheap_pmem;

/**
 * KERNEL MANAGEMENT
 */

void PageDirectory::init_kmem() {
	used_kernel_pmem = 0;
	used_kheap_pmem = 0;
	for(auto & entries : kernel_page_table_entries) for(auto & entry : entries) entry.value = 0;
	for(auto i = 0; i < 256; i++) {
		new (&kernel_page_tables[i]) PageTable(HIGHER_HALF + i * PAGE_SIZE * 1024,
													   &MemoryManager::inst().kernel_page_directory, false);

		kernel_page_tables[i].entries() = kernel_page_table_entries[i];
	}
	for(auto & physaddr : kernel_page_tables_physaddr) physaddr = 0;

	for(auto i = 0; i < 256; i++) {
		kernel_entries[i].value = 0;
		kernel_entries[i].data.present = true;
		kernel_entries[i].data.read_write = true;
		kernel_entries[i].data.user = false;
		kernel_entries[i].data.set_address((size_t)kernel_page_tables[i].entries() - HIGHER_HALF);
	}
}

void PageDirectory::map_kernel(MemoryRegion* text_region, MemoryRegion* data_region) {
	early_vmem_regions[0] = MemoryRegion(KERNEL_TEXT, KERNEL_TEXT_SIZE);
	early_vmem_regions[0].heap_allocated = false;
	early_vmem_regions[0].used = true;
	early_vmem_regions[0].next = &early_vmem_regions[1];

	early_vmem_regions[1] = MemoryRegion(KERNEL_DATA, KERNEL_DATA_SIZE);
	early_vmem_regions[1].heap_allocated = false;
	early_vmem_regions[1].used = true;
	early_vmem_regions[1].next = &early_vmem_regions[2];
	early_vmem_regions[1].prev = &early_vmem_regions[0];

	early_vmem_regions[2] = MemoryRegion(KERNEL_DATA_END, 0xFFFFFFFF - KERNEL_DATA_END);
	early_vmem_regions[2].heap_allocated = false;
	early_vmem_regions[2].prev = &early_vmem_regions[1];

	kernel_vmem_map = MemoryMap(PAGE_SIZE, &early_vmem_regions[0]);
	kernel_vmem_map.recalculate_memory_totals();
	used_kernel_pmem = early_vmem_regions[0].size;

	text_region->related = &early_vmem_regions[0];
	early_vmem_regions[0].related = text_region;
	LinkedMemoryRegion ktext(text_region, &early_vmem_regions[0]);
	k_map_region(ktext, false);

	data_region->related = &early_vmem_regions[1];
	early_vmem_regions[1].related = data_region;
	LinkedMemoryRegion kdata(data_region, &early_vmem_regions[1]);
	k_map_region(kdata, true);
}

void PageDirectory::k_map_region(const LinkedMemoryRegion& region, bool read_write) {
	//We have to do it this way instead of using LOCK() because vtables may not have been initialized yet
	MemoryManager::inst().kernel_page_directory._lock.acquire();

	MemoryRegion* physregion = region.phys;
	MemoryRegion* virtregion = region.virt;

	ASSERT(physregion->size == virtregion->size);
	ASSERT(virtregion->start >= HIGHER_HALF);

	size_t num_pages = physregion->size / PAGE_SIZE;
	size_t start_vpage = (virtregion->start - HIGHER_HALF) / PAGE_SIZE;
	size_t start_ppage = physregion->start / PAGE_SIZE;

	for(size_t page_index = 0; page_index < num_pages; page_index++) {
		size_t vpage = page_index + start_vpage;
		size_t directory_index = (vpage / 1024) % 1024;

		//The index into the page table of this page
		size_t table_index = vpage % 1024;
		//Set up the pagetable entry
		PageTable::Entry *entry = &kernel_page_tables[directory_index].entries()[table_index];
		entry->data.present = true;
		entry->data.read_write = read_write;
		entry->data.user = false;
		entry->data.set_address((start_ppage + page_index) * PAGE_SIZE);

		MemoryManager::inst().invlpg((void*)(virtregion->start + page_index * PAGE_SIZE));
	}

	MemoryManager::inst().kernel_page_directory._lock.release();
}

void PageDirectory::k_unmap_region(const LinkedMemoryRegion& region) {
	MemoryRegion* vregion = region.virt;
	size_t num_pages = vregion->size / PAGE_SIZE;
	size_t start_page = (vregion->start - HIGHER_HALF) / PAGE_SIZE;
	for(auto page = start_page; page < start_page + num_pages; page++) {
		size_t directory_index = (page / 1024) % 1024;
		size_t table_index = page % 1024;
		PageTable::Entry *table = &kernel_page_tables[directory_index].entries()[table_index];
		table->value = 0;
	}
}

LinkedMemoryRegion PageDirectory::k_map_physical_region(MemoryRegion* physregion, bool read_write) {
	LOCK(MemoryManager::inst().kernel_page_directory._lock);
	//First, try allocating a region of virtual memory.
	MemoryRegion* virtregion = kernel_vmem_map.allocate_region(physregion->size);
	if(!virtregion) {
		PANIC("KRNL_NO_VMEM_SPACE", "The kernel could not allocate a vmem region.");
	}

	//Then, map it.
	LinkedMemoryRegion reg(physregion, virtregion);
	k_map_region(reg, read_write);
	return reg;
}

void PageDirectory::k_free_virtual_region(LinkedMemoryRegion region) {
	k_unmap_region(region);
	kernel_vmem_map.free_region(region.virt);
}

LinkedMemoryRegion PageDirectory::k_alloc_region(size_t mem_size) {
	LOCK(MemoryManager::inst().kernel_page_directory._lock);

	//First, try allocating a region of virtual memory.
	MemoryRegion* vmem_region = kernel_vmem_map.allocate_region(mem_size);
	if(!vmem_region) {
		PANIC("KRNL_NO_VMEM_SPACE", "The kernel could not allocate a vmem region.");
	}

	//Next, try allocating the physical pages.
	ASSERT(false); // TODO
//	MemoryRegion* pmem_region = MemoryManager::inst().pmem_map().allocate_region(mem_size);
//	if(!pmem_region) {
//		PANIC("NO_MEM", "There's no more physical memory left.");
//	}
//
//	used_kernel_pmem += pmem_region->size;
//
//	//Finally, map the pages.
//	pmem_region->related = vmem_region;
//	vmem_region->related = pmem_region;
//	LinkedMemoryRegion region(pmem_region, vmem_region);
//	k_map_region(region, true);
//
//	memset((void*)vmem_region->start, 0, vmem_region->size);
//
//	return region;
}

bool new_heap_region = false;
bool allocing_new_heap_region = false;
MemoryRegion first_kmalloc_vregion, first_kmalloc_pregion;
LinkedMemoryRegion next_kmalloc_region(nullptr, nullptr);

void* PageDirectory::k_alloc_region_for_heap(size_t mem_size) {
	LOCK(MemoryManager::inst().kernel_page_directory._lock);
	if(allocing_new_heap_region)
		PANIC("KRNL_HEAP_FAIL", "The kernel tried to allocate more heap memory while already doing so.");

	//Use the first regions if we need to
	MemoryRegion *vregion_storage, *pregion_storage;
	if(next_kmalloc_region.virt == nullptr) {
		first_kmalloc_vregion.heap_allocated = false;
		first_kmalloc_pregion.heap_allocated = false;
		vregion_storage = &first_kmalloc_vregion;
		pregion_storage = &first_kmalloc_pregion;
	} else {
		vregion_storage = next_kmalloc_region.virt;
		pregion_storage = next_kmalloc_region.phys;
	}

	auto* vmem_region = kernel_vmem_map.allocate_region(mem_size, vregion_storage);
	if(!vmem_region) {
		PANIC("KRNL_NO_VMEM_SPACE", "The kernel could not allocate a vmem region for the heap.");
	}

	ASSERT(false); // TODO
//	auto* pmem_region = MemoryManager::inst().pmem_map().allocate_region(mem_size, pregion_storage);
//	if(!pmem_region) {
//		PANIC("NO_MEM", "There's no more physical memory left.");
//	}
//
//	used_kernel_pmem += pmem_region->size;
//
//	//Finally, map the pages and zero out.
//	pmem_region->related = vmem_region;
//	vmem_region->related = pmem_region;
//	LinkedMemoryRegion region(pmem_region, vmem_region);
//	k_map_region(region, true);
//	memset((void*)vmem_region->start, 0x00, vmem_region->size);
//
//	//Set new_heap_region to true so we allocate a new one next time.
//	new_heap_region = true;
//
//	return (void*) region.virt->start;
}

void PageDirectory::k_after_alloc() {
	LOCK(MemoryManager::inst().kernel_page_directory._lock);
	if(new_heap_region) {
		new_heap_region = false;
		allocing_new_heap_region = true;
		next_kmalloc_region.virt = new MemoryRegion();
		next_kmalloc_region.phys = new MemoryRegion();
		allocing_new_heap_region = false;
	}
}

void PageDirectory::k_free_region(const LinkedMemoryRegion& region) {
	LOCK(MemoryManager::inst().kernel_page_directory._lock);
	k_unmap_region(region);
	kernel_vmem_map.free_region(region.virt);
	if(region.phys->reserved)
		return;

	used_kernel_pmem -= region.phys->size;
	ASSERT(false); // TODO
//	MemoryManager::inst().pmem_map().free_region(region.phys);
}

bool PageDirectory::k_free_region(void* virtaddr) {
	MemoryRegion* vregion = kernel_vmem_map.find_region((size_t) virtaddr);
	if(!vregion) return false;
	if(vregion->reserved) return false;
	if(!vregion->related)
		PANIC("VREGION_NO_RELATED", "A virtual kernel memory region had no corresponding physical region.");
	if(vregion->related->reserved) return false;

	LOCK(MemoryManager::inst().kernel_page_directory._lock);
	LinkedMemoryRegion region(vregion->related, vregion);
	k_unmap_region(region);
	kernel_vmem_map.free_region(region.virt);
	used_kernel_pmem -= region.phys->size;
	ASSERT(false); // TODO
//	MemoryManager::inst().pmem_map().free_region(region.phys);
	return true;
}

void* PageDirectory::k_mmap(size_t physaddr, size_t memsize, bool read_write) {
	LOCK(MemoryManager::inst().kernel_page_directory._lock);
	size_t paddr_pagealigned = (physaddr / PAGE_SIZE) * PAGE_SIZE;
	size_t psize = (((memsize + (physaddr - paddr_pagealigned)) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
	MemoryRegion pregion = MemoryRegion(paddr_pagealigned, psize);
	pregion.reserved = true;

	//First, find a block of $pages contiguous virtual pages in the kernel space
	MemoryRegion* vregion = kernel_vmem_map.allocate_region(memsize);
	if(!vregion)
		return nullptr;

	//Next, map the pages
	LinkedMemoryRegion region(&pregion, vregion);
	k_map_region(region, read_write);

	return (void*)(region.virt->start + (physaddr % PAGE_SIZE));
}

bool PageDirectory::k_munmap(void* virtaddr) {
	MemoryRegion* vregion = kernel_vmem_map.find_region((size_t) virtaddr);
	if(!vregion) return false;
	LOCK(MemoryManager::inst().kernel_page_directory._lock);
	LinkedMemoryRegion region(nullptr, vregion);
	k_unmap_region(region);
	kernel_vmem_map.free_region(region.virt);
	return true;
}

bool PageDirectory::k_is_mapped(size_t addr) {
	if(addr < HIGHER_HALF) { //Program space
	   return false;
	} else { //Kernel space
		size_t page = (addr - HIGHER_HALF) / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		if (!kernel_entries[directory_index].data.present) return false;
	}
	return true;
}

/**
 * PageDirectory Entry stuff
 */


void PageDirectory::Entry::Data::set_address(size_t address) {
	page_table_addr = address >> 12u;
}

size_t PageDirectory::Entry::Data::get_address() {
	return page_table_addr << 12u;
}

/**
 * PageDirectory stuff
 */


PageDirectory::PageDirectory(bool no_init): _vmem_map(PAGE_SIZE, no_init ? nullptr : new MemoryRegion(PAGE_SIZE, HIGHER_HALF - PAGE_SIZE)) {
	if(!no_init) {
		_entries = (Entry*) k_alloc_region(PAGE_SIZE).virt->start;
		update_kernel_entries();
	}
}

PageDirectory::~PageDirectory() {
	k_free_region(_entries); //Free entries

	//Free page tables
	for(auto & table : _page_tables)
		if(table)
			delete table;

	//Free memory
	MemoryRegion* cur = _vmem_map.first_region();
	while(cur) {
		if(cur->used && cur->related){
			if(cur->cow.marked_cow) {
				cur->related->cow_deref();
			} else if(cur->is_shm) {
				cur->related->shm_deref();
			} else if(!cur->reserved) {
				ASSERT(false); // TODO
//				MemoryManager::inst().pmem_map().free_region(cur->related);
			}
		}
		cur = cur->next;
	}
}

PageDirectory::Entry *PageDirectory::entries() {
	return _entries;
}

size_t PageDirectory::entries_physaddr() {
	return get_physaddr((size_t)_entries);
}

SpinLock& PageDirectory::lock() {
	return _lock;
}

PageDirectory::Entry &PageDirectory::operator[](int index) {
	return _entries[index];
}

void PageDirectory::map_region(const LinkedMemoryRegion& region, bool read_write) {
	LOCK(_lock);
	MemoryRegion* physregion = region.phys;
	MemoryRegion* virtregion = region.virt;

	ASSERT(physregion->size == virtregion->size);
	ASSERT(virtregion->start + virtregion->size <= HIGHER_HALF);

	size_t num_pages = physregion->size / PAGE_SIZE;
	size_t start_vpage = virtregion->start / PAGE_SIZE;
	size_t start_ppage = physregion->start / PAGE_SIZE;
	for(size_t page_index = 0; page_index < num_pages; page_index++) {
		size_t vpage = page_index + start_vpage;
		size_t directory_index = (vpage / 1024) % 1024;

		//If the page table for this page hasn't been alloc'd yet, alloc it
		if (!_page_tables[directory_index]){
			alloc_page_table(directory_index);
		}

		//Set up the pagetable entry
		size_t table_index = vpage % 1024;
		_page_tables_num_mapped[directory_index]--;
		PageTable::Entry *entry = &_page_tables[directory_index]->entries()[table_index];
		entry->data.present = true;
		entry->data.read_write = read_write;
		entry->data.user = true;
		entry->data.set_address((start_ppage + page_index) * PAGE_SIZE);

		MemoryManager::inst().invlpg((void *) (virtregion->start + page_index * PAGE_SIZE));
	}
}

void PageDirectory::unmap_region(const LinkedMemoryRegion& region) {
	LOCK(_lock);
	MemoryRegion* vregion = region.virt;
	size_t num_pages = vregion->size / PAGE_SIZE;
	size_t start_page = vregion->start / PAGE_SIZE;
	for(auto page = start_page; page < start_page + num_pages; page++) {
		size_t directory_index = (page / 1024) % 1024;
		size_t table_index = page % 1024;
		PageTable::Entry *table = &_page_tables[directory_index]->entries()[table_index];
		table->value = 0;
		_page_tables_num_mapped[directory_index]--;
		if (_page_tables_num_mapped[directory_index] == 0)
			dealloc_page_table(directory_index);

		MemoryManager::inst().invlpg((void *) (vregion->start + page * PAGE_SIZE));
	}
}

size_t PageDirectory::get_physaddr(size_t virtaddr) {
	if(virtaddr < HIGHER_HALF) { //Program space
		size_t page = virtaddr / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		if (!_entries[directory_index].data.present) return -1; //TODO: Log an error
		if (!_page_tables[directory_index]) return -1; //TODO: Log an error
		size_t table_index = page % 1024;
		size_t page_paddr = (_page_tables[directory_index])->entries()[table_index].data.get_address();
		return page_paddr + (virtaddr % PAGE_SIZE);
	} else { //Kernel space
		size_t page = (virtaddr - HIGHER_HALF) / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		if (!kernel_entries[directory_index].data.present) return -1; //TODO: Log an error
		size_t table_index = page % 1024;
		size_t page_paddr = (kernel_page_tables[directory_index])[table_index].data.get_address();
		return page_paddr + (virtaddr % PAGE_SIZE);
	}
}

size_t PageDirectory::get_physaddr(void *virtaddr) {
	return get_physaddr((size_t)virtaddr);
}

LinkedMemoryRegion PageDirectory::allocate_region(size_t mem_size, bool read_write) {
	LOCK(_lock);
	//First, try allocating a region of virtual memory.
	MemoryRegion *vmem_region = _vmem_map.allocate_region(mem_size);
	if (!vmem_region) {
		//TODO: Send a signal instead
		PANIC("NO_VMEM_SPACE", "A program ran out of vmem space.");
	}

	//Next, try allocating the physical pages.
	ASSERT(false); // TODO
//	MemoryRegion *pmem_region = MemoryManager::inst().pmem_map().allocate_region(mem_size);
//	if (!pmem_region) {
//		PANIC("NO_MEM", "There's no more physical memory left.");
//	}
//	_used_pmem += pmem_region->size;
//
//	//Finally, map the pages.
//	pmem_region->related = vmem_region;
//	vmem_region->related = pmem_region;
//	LinkedMemoryRegion region(pmem_region, vmem_region);
//	map_region(region, read_write);
//
//	if(is_mapped()) {
//		memset((void*)vmem_region->start, 0, vmem_region->size);
//	} else {
//		//If the region isn't mapped, we have to map it to the kernel temporarily to zero it out
//		auto kernel_region = k_map_physical_region(pmem_region, true);
//		memset((void*)kernel_region.virt->start, 0, vmem_region->size);
//		k_free_virtual_region(kernel_region);
//	}
//
//	return region;
}

LinkedMemoryRegion PageDirectory::allocate_stack_region(size_t mem_size, bool read_write) {
	LOCK(_lock);
	//First, try allocating a region of virtual memory.
	MemoryRegion *vmem_region = _vmem_map.allocate_stack_region(mem_size);
	if (!vmem_region) {
		//TODO: Send a signal instead
		PANIC("NO_VMEM_SPACE", "A program ran out of vmem space.");
	}

	//Next, try allocating the physical pages.
	ASSERT(false); //TODO
//	MemoryRegion *pmem_region = MemoryManager::inst().pmem_map().allocate_region(mem_size);
//	if (!pmem_region) {
//		PANIC("NO_MEM", "There's no more physical memory left.");
//	}
//	_used_pmem += pmem_region->size;
//
//	//Finally, map the pages.
//	pmem_region->related = vmem_region;
//	vmem_region->related = pmem_region;
//	LinkedMemoryRegion region(pmem_region, vmem_region);
//	map_region(region, read_write);
//
//	if(is_mapped()) {
//		memset((void*)vmem_region->start, 0, vmem_region->size);
//	} else {
//		//If the region isn't mapped, we have to map it to the kernel temporarily to zero it out
//		auto kernel_region = k_map_physical_region(pmem_region, true);
//		memset((void*)kernel_region.virt->start, 0, vmem_region->size);
//		k_free_virtual_region(kernel_region);
//	}
//
//	return region;
}

LinkedMemoryRegion PageDirectory::allocate_region(size_t vaddr, size_t mem_size, bool read_write) {
	LOCK(_lock);
	//First, try allocating a region of virtual memory.
	MemoryRegion *vmem_region = _vmem_map.allocate_region(vaddr, mem_size);
	if (!vmem_region)
		return {nullptr, nullptr};

	//Next, try allocating the physical pages.
	ASSERT(false);
	// TODO
//	MemoryRegion *pmem_region = MemoryManager::inst().pmem_map().allocate_region(mem_size);
//	if (!pmem_region) {
//		PANIC("NO_MEM", "There's no more physical memory left.");
//	}
//	_used_pmem += pmem_region->size;
//
//	//Finally, map the pages.
//	pmem_region->related = vmem_region;
//	vmem_region->related = pmem_region;
//	LinkedMemoryRegion region(pmem_region, vmem_region);
//	map_region(region, read_write);
//
//	if(is_mapped()) {
//		memset((void*)vmem_region->start, 0, vmem_region->size);
//	} else {
//		//If the region isn't mapped, we have to map it to the kernel temporarily to zero it out
//		auto kernel_region = k_map_physical_region(pmem_region, true);
//		memset((void*)kernel_region.virt->start, 0, vmem_region->size);
//		k_free_virtual_region(kernel_region);
//	}
//
//	return region;
}

void PageDirectory::free_region(const LinkedMemoryRegion& region) {
	LOCK(_lock);
	//Don't allow the freeing of shared memory regions
	if(region.virt->is_shm)
		return;

	//Unmap the region
	unmap_region(region);

	bool was_cow = region.virt->cow.marked_cow;
	_vmem_map.free_region(region.virt);

	//If the physical region is reserved (AKA memory-mapped hardware) don't mark it free
	if(region.phys->reserved)
		return;
	else if(was_cow) {
		//If the region is marked CoW, just dereference it
		region.phys->cow_deref();
	} else {
		_used_pmem -= region.phys->size;
		ASSERT(false); // TODO
//		MemoryManager::inst().pmem_map().free_region(region.phys);
	}
}

bool PageDirectory::free_region(size_t virtaddr, size_t size) {
	LOCK(_lock);
	//Find the appropriate region
	MemoryRegion* vregion = _vmem_map.find_region(virtaddr);
	if(!vregion) return false;
	if(!vregion->used) return false;
	if(vregion->is_shm) return false;
	if(!vregion->related)
		PANIC("VREGION_NO_RELATED", "A virtual program memory region had no corresponding physical region.");

	auto* pregion = vregion->related;
	if(pregion->is_shm) return false;
	if(pregion->reserved) return false;
	if(!vregion->cow.marked_cow) {
		//Split the physical region if it isn't CoW
		size_t pregion_split_start = pregion->start + (virtaddr - vregion->start);
		ASSERT(false); // TODO
//		pregion = MemoryManager::inst().pmem_map().split_region(pregion, pregion_split_start, size);
		if(!pregion) return false;
	}

	//Split the virtual region
	vregion = _vmem_map.split_region(vregion, virtaddr, size);
	if(!vregion)
		PANIC("VREGION_SPLIT_FAIL", "A virtual program memory region couldn't be split after its physical region was split.");

	//Unmap and free the regions
	LinkedMemoryRegion region(pregion, vregion);
	free_region(region);
	return true;
}

void* PageDirectory::mmap(size_t physaddr, size_t memsize, bool read_write) {
	LOCK(_lock);
	size_t paddr_pagealigned = (physaddr / PAGE_SIZE) * PAGE_SIZE;
	size_t psize = (((memsize + (physaddr - paddr_pagealigned)) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
	MemoryRegion pregion = MemoryRegion(paddr_pagealigned, psize);
	pregion.reserved = true;

	//First, find a block of $pages contiguous virtual pages in the program space
	MemoryRegion* vregion = _vmem_map.allocate_region(memsize);
	if(!vregion) {
		return nullptr;
	}

	vregion->reserved = true;

	//Next, map the pages
	LinkedMemoryRegion region(&pregion, vregion);
	map_region(region, read_write);

	return (void*)(region.virt->start + (physaddr % PAGE_SIZE));
}

bool PageDirectory::munmap(void* virtaddr) {
	LOCK(_lock);
	MemoryRegion* vregion = _vmem_map.find_region((size_t) virtaddr);
	if(!vregion) return false;
	if(!vregion->reserved) return false;
	LinkedMemoryRegion region(nullptr, vregion);
	unmap_region(region);
	_vmem_map.free_region(region.virt);
	return true;
}

/// SHARED MEMORY STUFF
kstd::map<int, MemoryRegion*> PageDirectory::m_shared_regions;
int PageDirectory::m_shared_id_counter = 1;
SpinLock PageDirectory::m_shared_region_lock;

ResultRet<LinkedMemoryRegion> PageDirectory::create_shared_region(size_t vaddr, size_t mem_size, pid_t pid) {
	LOCK(_lock);

	if(!mem_size)
		mem_size = 1; //Make sure we're allocating at least one byte...

	//First, allocate the region
	LinkedMemoryRegion region;
	if(vaddr)
		region = allocate_region(vaddr, mem_size, true);
	else
		region = allocate_region(mem_size, true);

	if(!region.virt)
		return Result(-ENOMEM);

	//Then, set up the regions
	LOCK_N(m_shared_region_lock, __shmemlock);

	region.virt->is_shm = true;
	region.phys->is_shm = true;
	region.phys->shm_id = m_shared_id_counter++;
	region.virt->shm_id = region.phys->shm_id;
	region.phys->shm_refs = 1;
	region.phys->shm_owner = pid;
	region.phys->shm_allowed = new kstd::map<pid_t, MemoryRegion::ShmPermissions>();
	region.phys->shm_allowed->insert({pid, {true, true}});

	//Register the shared region
	m_shared_regions.insert({region.phys->shm_id, region.phys});

	//Add it to the attached shm regions
	_attached_shm_regions.push_back(region.virt);

	//Increase used shared memory
	_used_shmem += region.virt->size;

	return region;
}

ResultRet<LinkedMemoryRegion> PageDirectory::attach_shared_region(int id, size_t vaddr, pid_t pid) {
	LOCK(_lock);

	//Check if we've already attached the region
	for(auto i = 0; i < _attached_shm_regions.size(); i++) {
		if (_attached_shm_regions[i]->shm_id == id) {
			return LinkedMemoryRegion(_attached_shm_regions[i], _attached_shm_regions[i]->related);
		}
	}

	//Find the shared region
	LOCK_N(m_shared_region_lock, __shmemlock);
	MemoryRegion* pmem_region = nullptr;
	auto* reg = m_shared_regions.find_node(id);
	if(reg)
		pmem_region = reg->data.second;

	//If it doesn't exist or isn't a shared memory region, return ENOENT
	if(!pmem_region || !pmem_region->shm_allowed)
		return Result(-ENOENT);

	//Double check our permissions
	LOCK_N(pmem_region->lock, __pmem_lock);
	if(!pmem_region->shm_allowed->contains(pid))
		return Result(-ENOENT);
	bool write = (*pmem_region->shm_allowed)[pid].write;

	//Then, allocate a vmem region for it
	MemoryRegion* vmem_region = nullptr;
	size_t vaddr_pagealigned = (vaddr / PAGE_SIZE) * PAGE_SIZE;
	if(vaddr) {
		//Allocate the region at the specified address
		vmem_region = _vmem_map.allocate_region(vaddr_pagealigned, pmem_region->size);
	} else {
		//Allocate a new region
		vmem_region = _vmem_map.allocate_region(pmem_region->size);
	}

	//If we failed to allocate the virtual region, error out
	if(!vmem_region)
		return Result(vaddr ? -EEXIST : -ENOMEM);

	//Set up the vmem region
	vmem_region->is_shm = true;
	vmem_region->shm_id = id;

	//Finally, map the region
	vmem_region->related = pmem_region;
	LinkedMemoryRegion linked_region(pmem_region, vmem_region);
	map_region(linked_region, write);
	_used_pmem += pmem_region->size;
	_used_shmem += pmem_region->size;
	_attached_shm_regions.push_back(vmem_region);
	pmem_region->shm_ref();

	return linked_region;
}

Result PageDirectory::detach_shared_region(int id) {
	LOCK(_lock);

	//Find the virtual region in question and if it doesn't exist, see if it's in the pending regions
	MemoryRegion* vreg = _vmem_map.find_shared_region(id);

	//Decrease mem usage
	_used_pmem -= vreg->size;
	_used_shmem -= vreg->size;

	//Remove the region from the attached shm regions list
	for(auto i = 0; i < _attached_shm_regions.size(); i++) {
		if(_attached_shm_regions[i]->is_shm && _attached_shm_regions[i]->shm_id == id) {
			_attached_shm_regions.erase(i);
			break;
		}
	}

	//Unmap the region and decrease the reference count
	LinkedMemoryRegion reg(vreg->related, vreg);
	unmap_region(reg);
	vreg->related->shm_deref();
	vreg->related = nullptr;
	vreg->is_shm = false;
	vreg->shm_id = 0;
	_vmem_map.free_region(vreg);

	return Result(SUCCESS);
}

Result PageDirectory::allow_shared_region(int id, pid_t called_pid, pid_t pid, bool write, bool share) {
	LOCK(_lock);
	LOCK_N(m_shared_region_lock, __shmemlock);

	//Find the physical region in question and if it doesn't exist, return EINVAL
	//We could have permission to share it but have not attached it yet, so we search all of pmem
	auto* preg = m_shared_regions.find_node(id);
	if(!preg)
		return Result(-EINVAL);
	auto* phys = preg->data.second;
	LOCK_N(phys->lock, __physlock);

	//If we don't have share permissions, or we're trying to share with permissions we don't have, return EPERM
	if(!phys->shm_allowed->contains(called_pid))
		return Result(-EPERM);
	auto our_perms = (*phys->shm_allowed)[called_pid];
	if(!our_perms.share || (!our_perms.write && write))
		return Result(-EPERM);

	//Make sure the process we want exists
	auto proc_res = TaskManager::process_for_pid(pid);
	if(proc_res.is_error())
		return Result(-ENOENT);
	auto proc = proc_res.value();

	//Update the permissions
	LOCK_N(proc->page_directory()->_lock, __proc_pd_lock);
	phys->shm_allowed->insert({pid, {write, share}});

	return Result(SUCCESS);
}

void PageDirectory::deregister_shared_region(MemoryRegion* region) {
	LOCK(m_shared_region_lock);
	m_shared_regions.erase(region->shm_id);
}

MemoryMap& PageDirectory::vmem_map() {
	return _vmem_map;
}

PageTable *PageDirectory::alloc_page_table(size_t tables_index) {
	LOCK(_lock);
	//If one was already allocated, return it
	if(_page_tables[tables_index])
		return _page_tables[tables_index];

	auto *table = new PageTable(tables_index * PAGE_SIZE * 1024, this);
	_page_tables[tables_index] = table;
	PageDirectory::Entry *direntry = &_entries[tables_index];
	direntry->data.set_address(get_physaddr(table->entries()));
	direntry->data.present = true;
	direntry->data.user = true;
	direntry->data.read_write = true;
	return table;
}

void PageDirectory::dealloc_page_table(size_t tables_index) {
	LOCK(_lock);
	if(!_page_tables[tables_index])
		return;
	delete _page_tables[tables_index];
	_page_tables[tables_index] = nullptr;
	_entries[tables_index].value = 0;
}

void PageDirectory::update_kernel_entries() {
	for(auto i = 768; i < 1024; i++){
		auto ki = i - 768;
		_entries[i].value = kernel_entries[ki].value;
	}
}

void PageDirectory::set_entries(Entry* entries) {
	LOCK(_lock);
	_entries = entries;
}

bool PageDirectory::is_mapped(size_t vaddr, bool write) {
	LOCK(_lock);
	if(vaddr < HIGHER_HALF) { //Program space
		size_t page = vaddr / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		if (!_entries[directory_index].data.present) return false;
		if (!_page_tables[directory_index]) return false;
		auto& entry = _page_tables[directory_index]->entries()[page % 1024];
		if(!entry.data.present)
			return false;
		if(write) {
			if(!entry.data.read_write) {
				// If we need to check for write perms and the page is not writable, check for CoW
				auto* region = _vmem_map.find_region(vaddr);
				if(!region || !region->cow.marked_cow)
					return false;
			}
		}
		return true;
	} else { //Kernel space
		size_t page = (vaddr - HIGHER_HALF) / PAGE_SIZE;
		size_t directory_index = (page / 1024) % 1024;
		if (!kernel_entries[directory_index].data.present) return false;
		auto& entry = kernel_page_tables[directory_index][page % 1024];;
		return entry.data.present && (!write || entry.data.read_write);
	}
}

void PageDirectory::fork_from(PageDirectory *parent, pid_t parent_pid, pid_t new_pid) {
	LOCK(parent->_lock);
	//Iterate through every entry of the page directory we're copying from
	MemoryRegion* parent_region = parent->_vmem_map.first_region();
	while(parent_region) {
		if(parent_region->used) {
			auto* new_region = _vmem_map.allocate_region(parent_region->start, parent_region->size);
			if(!new_region)
				PANIC("COW_FAILED", "CoW failed to allocate a vmem region.");

			if(parent_region->is_shm) {
				//If the region is shared, increase the number of refs on it and map it with the correct permissions.
				auto* shm_region = parent_region->related;
				shm_region->lock.acquire();
				shm_region->shm_ref();

				//Figure out the permission
				if(!shm_region->shm_allowed->contains(parent_pid)) {
					printf("Permissions of shared memory region at 0x%x set up incorrectly!", shm_region->start);
					_vmem_map.free_region(new_region);
					parent_region = parent_region->next;
					shm_region->lock.release();
					continue;
				}

				//Add a permissions entry to the region and map it
				auto perms = (*shm_region->shm_allowed)[parent_pid];
				shm_region->shm_allowed->insert({new_pid, perms});
				new_region->related = shm_region;
				new_region->is_shm = true;
				new_region->shm_id = shm_region->shm_id;
				map_region(LinkedMemoryRegion(shm_region, new_region), perms.write);
				shm_region->lock.release();
				_attached_shm_regions.push_back(new_region);
			} else if(parent_region->reserved) {
				//This is reserved memory (AKA memory-mapped hardware or something), so don't map it to the child
				_vmem_map.free_region(new_region);
			} else {
				parent_region->related->lock.acquire();
				if(parent_region->cow.marked_cow) {
					//If the region is already marked cow, increase the number of refs by one.
					parent_region->related->cow.num_refs++;
				} else {
					//Otherwise, set it to 2
					parent_region->related->cow.num_refs = 2;
				}
				parent_region->related->lock.release();

				parent_region->cow.marked_cow = true;
				new_region->cow.marked_cow = true;
				new_region->related = parent_region->related;
				parent_region->related->related = nullptr;

				//Mark the page table entries in the parent read-only
				size_t num_pages = parent_region->size / PAGE_SIZE;
				size_t start_vpage = parent_region->start / PAGE_SIZE;
				for(size_t page_index = 0; page_index < num_pages; page_index++) {
					size_t vpage = page_index + start_vpage;
					parent->_page_tables[(vpage / 1024) % 1024]->entries()[vpage % 1024].data.read_write = false;
				}

				//Map the page table entries in child as read-only
				map_region(LinkedMemoryRegion(new_region->related, new_region), false);
			}
		}
		parent_region = parent_region->next;
	}
}

bool PageDirectory::try_cow(size_t virtaddr) {
	LOCK(_lock);

	auto* region = _vmem_map.find_region(virtaddr);
	if(!region || !region->cow.marked_cow) return false;

	TaskManager::Disabler disabler;
	ASSERT(is_mapped());

	//Allocate a temporary kernel region to copy the memory into
	LinkedMemoryRegion tmp_region = k_alloc_region(region->size);
	if(!tmp_region.virt)
		PANIC("COW_COPY_FAIL", "CoW failed to allocate a memory region to copy.");

	//Copy the region into the buffer
	memcpy((void*)tmp_region.virt->start, (void*)region->start, region->size);

	//Unmap the buffer region from the kernel
	k_free_virtual_region(tmp_region);

	//Reduce reference count of physical region and map new physical region to virtual region
	region->related->cow_deref();
	region->cow.marked_cow = false;

	region->related = tmp_region.phys;
	tmp_region.phys->related = region;
	LinkedMemoryRegion new_region(tmp_region.phys, region);
	map_region(new_region, true);
	_used_pmem += tmp_region.phys->size;

	return true;
}

size_t PageDirectory::used_pmem() {
	return _used_pmem;
}

size_t PageDirectory::used_vmem() {
	return _vmem_map.used_memory();
}

size_t PageDirectory::used_shmem() {
	return _used_shmem;
}

bool PageDirectory::is_mapped() {
	size_t current_page_directory;
	asm volatile("mov %%cr3, %0" : "=r"(current_page_directory));
	return current_page_directory == entries_physaddr();

}
