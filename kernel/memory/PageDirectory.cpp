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
uint8_t __kernel_vmem_map_storage[sizeof(VMMap)];
uint8_t __early_vmem_regions_storage[sizeof(MemoryRegion) * 3];

PageDirectory::Entry (&PageDirectory::kernel_entries)[256] = (PageDirectory::Entry(&)[256]) *__kernel_entries_storage;
PageTable (&PageDirectory::kernel_page_tables)[256] = (PageTable(&)[256]) *__kernel_page_tables_storage;
size_t PageDirectory::kernel_page_tables_physaddr[1024];
VMMap& PageDirectory::kernel_vmem_map = (VMMap&) *__kernel_vmem_map_storage;
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

	kernel_vmem_map = VMMap(PAGE_SIZE, &early_vmem_regions[0]);
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

void PageDirectory::map(kstd::shared_ptr<VMRegion> region) {
	LOCK(_lock);

	PageIndex start_vpage = region->start() / PAGE_SIZE;
	size_t num_pages = region->size() / PAGE_SIZE;
	auto prot = region->prot();
	ASSERT(prot.read);

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
		entry->data.read_write = prot.write;
		entry->data.user = true;
		entry->data.set_address(region->object()->physical_page(page_index).index() * PAGE_SIZE);

		MemoryManager::inst().invlpg((void *) (vpage * PAGE_SIZE));
	}
}

void PageDirectory::unmap(kstd::shared_ptr<VMRegion> region) {
	LOCK(_lock);

	PageIndex start_vpage = region->start() / PAGE_SIZE;
	size_t num_pages = region->size() / PAGE_SIZE;

	for(size_t page_index = 0; page_index < num_pages; page_index++) {
		PageIndex page = start_vpage + page_index;
		size_t directory_index = (page / 1024) % 1024;
		size_t table_index = page % 1024;
		PageTable::Entry *table = &_page_tables[directory_index]->entries()[table_index];
		table->value = 0;
		_page_tables_num_mapped[directory_index]--;
		if (_page_tables_num_mapped[directory_index] == 0)
			dealloc_page_table(directory_index);

		MemoryManager::inst().invlpg((void *) (page * PAGE_SIZE));
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
			if(!entry.data.read_write)
				return false;
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

bool PageDirectory::is_mapped() {
	size_t current_page_directory;
	asm volatile("mov %%cr3, %0" : "=r"(current_page_directory));
	return current_page_directory == entries_physaddr();
}


