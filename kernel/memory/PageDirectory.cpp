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

#include <kernel/tasking/TaskManager.h>
#include <kernel/memory/PageDirectory.h>
#include "PageTable.h"

PageDirectory::Entry PageDirectory::kernel_entries[256];
PageTable PageDirectory::kernel_page_tables[256];
size_t PageDirectory::kernel_page_tables_physaddr[1024];
MemoryMap PageDirectory::kernel_vmem_map(0, nullptr);
MemoryRegion PageDirectory::early_vmem_regions[2] = {{0, 0}, {0, 0}};
PageTable::Entry kernel_page_table_entries[256][1024] __attribute__((aligned(4096)));
MemoryRegion vmem_region_buffer(0,0);
MemoryRegion pmem_region_buffer(0,0);
bool region_buffer_needs_attention = false;

/**
 * KERNEL MANAGEMENT
 */

void PageDirectory::init_kmem() {
	for(auto & entries : kernel_page_table_entries) for(auto & entry : entries) entry.value = 0;
	for(auto i = 0; i < 256; i++) {
		new (&kernel_page_tables[i]) PageTable(HIGHER_HALF + i * PAGE_SIZE * 1024,
													   &Memory::kernel_page_directory, false);

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

void PageDirectory::map_kernel(MemoryRegion* kernel_pmem_region) {
	early_vmem_regions[0] = MemoryRegion(KERNEL_START, KERNEL_SIZE_PAGES * PAGE_SIZE);
	early_vmem_regions[0].heap_allocated = false;
	early_vmem_regions[0].used = true;
	early_vmem_regions[0].next = &early_vmem_regions[1];

	early_vmem_regions[1] = MemoryRegion(KERNEL_START + early_vmem_regions[0].size, 0xFFFFFFFF - (KERNEL_START + early_vmem_regions[0].size));
	early_vmem_regions[1].heap_allocated = false;
	early_vmem_regions[1].prev = &early_vmem_regions[0];

	kernel_vmem_map = MemoryMap(PAGE_SIZE, &early_vmem_regions[0]);
	kernel_vmem_map.recalculate_memory_totals();

	LinkedMemoryRegion kregion(kernel_pmem_region, &early_vmem_regions[0]);

	k_map_region(kregion, true);
}

void PageDirectory::k_map_region(const LinkedMemoryRegion& region, bool read_write) {
	MemoryRegion* physregion = region.phys;
	MemoryRegion* virtregion = region.virt;

	ASSERT(physregion->size == virtregion->size);
	ASSERT(virtregion->start >= HIGHER_HALF);

	size_t num_pages = physregion->size / PAGE_SIZE;
	size_t start_vpage = (virtregion->start - HIGHER_HALF) / PAGE_SIZE;
	size_t start_ppage = physregion->start / PAGE_SIZE;
	for(auto page_index = 0; page_index < num_pages; page_index++) {
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

		Memory::invlpg((void*)(virtregion->start + page_index * PAGE_SIZE));
	}
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

LinkedMemoryRegion PageDirectory::k_alloc_region(size_t mem_size) {
	//First, try allocating a region of virtual memory.
	MemoryRegion* vmem_region = kernel_vmem_map.allocate_region(mem_size);
	if(!vmem_region) {
		PANIC("KRNL_NO_VMEM_SPACE", "The kernel could not allocate a vmem region.", true);
	}

	//Next, try allocating the physical pages.
	MemoryRegion* pmem_region = Memory::pmem_map().allocate_region(mem_size);
	if(!pmem_region) {
		PANIC("NO_MEM", "There's no more physical memory left.", true);
	}

	//Finally, map the pages.
	LinkedMemoryRegion region(pmem_region, vmem_region);
	k_map_region(region, true);

	memset((void*)vmem_region->start, 0, vmem_region->size);

	return region;
}

void* PageDirectory::k_alloc_region_for_heap(size_t mem_size) {
	region_buffer_needs_attention = true;

	MemoryRegion* vmem_region = kernel_vmem_map.allocate_region(mem_size, &vmem_region_buffer);
	vmem_region_buffer.heap_allocated = false;
	if(!vmem_region) {
		Memory::kernel_page_directory.dump();
		PANIC("KRNL_NO_VMEM_SPACE", "The kernel could not allocate a vmem region for the heap.", true);
	}

	//Next, try allocating the physical pages.
	MemoryRegion* pmem_region = Memory::pmem_map().allocate_region(mem_size, &pmem_region_buffer);
	pmem_region_buffer.heap_allocated = false;
	if(!pmem_region) {
		PANIC("NO_MEM", "There's no more physical memory left.", true);
	}

	//Finally, map the pages and zero out.
	LinkedMemoryRegion region(pmem_region, vmem_region);
	k_map_region(region, true);
	memset((void*)vmem_region->start, 0x00, vmem_region->size);

	return (void*) region.virt->start;
}

void PageDirectory::k_after_alloc() {
	if(region_buffer_needs_attention) {
		region_buffer_needs_attention = false;
		auto* new_vmem = new MemoryRegion(vmem_region_buffer);
		auto* new_pmem = new MemoryRegion(pmem_region_buffer);
		new_vmem->related = new_pmem;
		new_pmem->related = new_vmem;
		kernel_vmem_map.replace_entry(&vmem_region_buffer, new_vmem);
		Memory::pmem_map().replace_entry(&pmem_region_buffer, new_pmem);
	}
}

void PageDirectory::k_free_region(const LinkedMemoryRegion& region) {
	k_unmap_region(region);
	kernel_vmem_map.free_region(region.virt);
	Memory::pmem_map().free_region(region.phys);
}

bool PageDirectory::k_free_region(void* virtaddr) {
	MemoryRegion* vregion = kernel_vmem_map.find_region((size_t) virtaddr);
	if(!vregion) return false;
	if(!vregion->related) PANIC("VREGION_NO_RELATED", "A virtual kernel memory region had no corresponding physical region.", true);
	LinkedMemoryRegion region(vregion->related, vregion);
	k_unmap_region(region);
	kernel_vmem_map.free_region(region.virt);
	Memory::pmem_map().free_region(region.phys);
	return true;
}

void* PageDirectory::k_mmap(size_t physaddr, size_t memsize, bool read_write) {
	size_t paddr_pagealigned = (physaddr / PAGE_SIZE) * PAGE_SIZE;
	size_t psize = (((memsize + (physaddr - paddr_pagealigned)) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
	MemoryRegion pregion = MemoryRegion(paddr_pagealigned, psize);

	//First, find a block of $pages contiguous virtual pages in the kernel space
	MemoryRegion* vregion = kernel_vmem_map.allocate_region(memsize);
	if(!vregion) {
		return nullptr;
	}

	//Next, map the pages
	LinkedMemoryRegion region(&pregion, vregion);
	k_map_region(region, read_write);

	return (void*)(region.virt->start + (physaddr % PAGE_SIZE));
}

bool PageDirectory::k_munmap(void* virtaddr) {
	MemoryRegion* vregion = kernel_vmem_map.find_region((size_t) virtaddr);
	if(!vregion) return false;
	LinkedMemoryRegion region(nullptr, vregion);
	k_unmap_region(region);
	kernel_vmem_map.free_region(region.virt);
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


PageDirectory::PageDirectory(): _vmem_map(PAGE_SIZE, new MemoryRegion(0, HIGHER_HALF)) {
	_entries = (Entry*) k_alloc_region(PAGE_SIZE).virt->start;
	update_kernel_entries();
}

PageDirectory::~PageDirectory() {
	k_free_region(_entries); //Free entries

	//Free page tables
	for(auto & table : _page_tables) if(table) {
		delete table;
	}

	MemoryRegion* cur = _vmem_map.first_region();
	while(cur) {
		if(cur->used && cur->related){
			if(cur->cow.marked_cow) {
				cur->related->cow_deref();
			} else {
				Memory::pmem_map().free_region(cur->related);
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

PageDirectory::Entry &PageDirectory::operator[](int index) {
	return _entries[index];
}

void PageDirectory::map_region(const LinkedMemoryRegion& region, bool read_write) {
	MemoryRegion* physregion = region.phys;
	MemoryRegion* virtregion = region.virt;

	ASSERT(physregion->size == virtregion->size);
	ASSERT(virtregion->start + virtregion->size <= HIGHER_HALF);

	size_t num_pages = physregion->size / PAGE_SIZE;
	size_t start_vpage = virtregion->start / PAGE_SIZE;
	size_t start_ppage = physregion->start / PAGE_SIZE;
	for(auto page_index = 0; page_index < num_pages; page_index++) {
		size_t vpage = page_index + start_vpage;
		size_t directory_index = (vpage / 1024) % 1024;

		//If the page table for this page hasn't been alloc'd yet, alloc it
		if (!_page_tables[directory_index]){
			alloc_page_table(directory_index);
		}

		//Set up the pagetable entry
		size_t table_index = vpage % 1024;
		_page_tables_num_mapped[directory_index]++;
		PageTable::Entry *entry = &_page_tables[directory_index]->entries()[table_index];
		entry->data.present = true;
		entry->data.read_write = read_write;
		entry->data.user = true;
		entry->data.set_address((start_ppage + page_index) * PAGE_SIZE);

		Memory::invlpg((void *) (virtregion->start + page_index * PAGE_SIZE));
	}
}

void PageDirectory::unmap_region(const LinkedMemoryRegion& region) {
	MemoryRegion* vregion = region.virt;
	size_t num_pages = vregion->size / PAGE_SIZE;
	size_t start_page = vregion->start/ PAGE_SIZE;
	for(auto page = start_page; page < start_page + num_pages; page++) {
		size_t directory_index = (page / 1024) % 1024;
		size_t table_index = page % 1024;
		_page_tables_num_mapped[directory_index]--;
		PageTable::Entry *table = &_page_tables[directory_index]->entries()[table_index];
		table->value = 0;
		if (_page_tables_num_mapped[directory_index] == 0)
			dealloc_page_table(directory_index);
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
	//First, try allocating a region of virtual memory.
	MemoryRegion *vmem_region = _vmem_map.allocate_region(mem_size);
	if (!vmem_region) {
		//TODO: Send a signal instead
		PANIC("NO_VMEM_SPACE", "A program ran out of vmem space.", true);
	}

	//Next, try allocating the physical pages.
	MemoryRegion *pmem_region = Memory::pmem_map().allocate_region(mem_size);
	if (!pmem_region) {
		PANIC("NO_MEM", "There's no more physical memory left.", true);
	}

	//Finally, map the pages.
	LinkedMemoryRegion region(pmem_region, vmem_region);
	map_region(region, read_write);

	memset((void*)vmem_region->start, 0, vmem_region->size);

	return region;
}

LinkedMemoryRegion PageDirectory::allocate_region(size_t vaddr, size_t mem_size, bool read_write) {
	//First, try allocating a region of virtual memory.
	MemoryRegion *vmem_region = _vmem_map.allocate_region(vaddr, mem_size);
	if (!vmem_region) {
		//TODO: Send a signal instead
		PANIC("NO_VMEM_SPACE", "A program ran out of vmem space.", true);
	}

	//Next, try allocating the physical pages.
	MemoryRegion *pmem_region = Memory::pmem_map().allocate_region(mem_size);
	if (!pmem_region) {
		PANIC("NO_MEM", "There's no more physical memory left.", true);
	}

	//Finally, map the pages.
	LinkedMemoryRegion region(pmem_region, vmem_region);
	map_region(region, read_write);

	memset((void*)vmem_region->start, 0, vmem_region->size);

	return region;
}

void PageDirectory::free_region(const LinkedMemoryRegion& region) {
	unmap_region(region);
	_vmem_map.free_region(region.virt);
	Memory::pmem_map().free_region(region.phys);
}

bool PageDirectory::free_region(void* virtaddr) {
	MemoryRegion* vregion = _vmem_map.find_region((size_t) virtaddr);
	if(!vregion) return false;
	if(!vregion->related) PANIC("VREGION_NO_RELATED", "A virtual program memory region had no corresponding physical region.", true);
	LinkedMemoryRegion region(vregion->related, vregion);
	unmap_region(region);
	_vmem_map.free_region(region.virt);
	Memory::pmem_map().free_region(region.phys);
	return true;
}

MemoryMap& PageDirectory::vmem_map() {
	return _vmem_map;
}

PageTable *PageDirectory::alloc_page_table(size_t tables_index) {
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
	_entries = entries;
}

void PageDirectory::fork_from(PageDirectory *parent) {
	//Iterate through every entry of the page directory we're copying from
	MemoryRegion* parent_region = parent->_vmem_map.first_region();
	while(parent_region) {
		if(parent_region->used) {
			auto* new_region = _vmem_map.allocate_region(parent_region->start, parent_region->size);
			if(!new_region)
				PANIC("COW_FAILED", "CoW failed to allocate a vmem region.", true);

			//If the region is already marked cow, increase the number of refs by one. Otherwise, set it to 2
			if(parent_region->cow.marked_cow)
				parent_region->related->cow.num_refs += 1;
			else
				parent_region->related->cow.num_refs = 2;

			parent_region->cow.marked_cow = true;
			new_region->cow.marked_cow = true;
			new_region->related = parent_region->related;

			//Mark the page table entries in the parent read-only
			size_t num_pages = parent_region->size / PAGE_SIZE;
			size_t start_vpage = parent_region->start / PAGE_SIZE;
			for(auto page_index = 0; page_index < num_pages; page_index++) {
				size_t vpage = page_index + start_vpage;
				parent->_page_tables[(vpage / 1024) % 1024]->entries()[vpage % 1024].data.read_write = false;
			}

			//Map the page table entries in child as read-only
			map_region(LinkedMemoryRegion(new_region->related, new_region), false);


		}
		parent_region = parent_region->next;
	}
}

bool PageDirectory::try_cow(size_t virtaddr) {
	auto* region = _vmem_map.find_region(virtaddr);
	if(!region || !region->cow.marked_cow) return false;

	//Allocate a temporary kernel region to copy the memory into
	LinkedMemoryRegion tmp_region = k_alloc_region(region->size);
	if(!tmp_region.virt) PANIC("COW_COPY_FAIL", "CoW failed to allocate a memory region to copy.", true);

	//Copy the region into the buffer
	memcpy((void*)tmp_region.virt->start, (void*)region->start, region->size);

	//Unmap the buffer region from the kernel
	k_unmap_region(tmp_region);
	kernel_vmem_map.free_region(tmp_region.virt);

	//Reduce reference count of physical region and map new physical region to virtual region
	region->related->cow_deref();
	region->cow.marked_cow = false;
	LinkedMemoryRegion new_region(tmp_region.phys, region);
	map_region(new_region, true);

	return true;
}

size_t PageDirectory::used_pmem() {
	return 0; //TODO
	//return (_personal_pmem_bitmap.used_pages() * PAGE_SIZE) / 1024;
}

void PageDirectory::dump() {
	MemoryRegion* cur = Memory::pmem_map().first_region();
	printf("\nPHYSICAL:\n");
	while(cur) {
		printf("%x %x %d | %x\n", cur->start, cur->size, cur->used, cur->related ? cur->related->start : -1);
		cur = cur->next;
	}
	printf("\nKERNEL:\n");
	cur = kernel_vmem_map.first_region();
	while(cur) {
		printf("%x %x %d | %x\n", cur->start, cur->size, cur->used, cur->related ? cur->related->start : -1);
		cur = cur->next;
	}
	if(!_vmem_map.first_region()) {
		printf("\n");
		return;
	}
	printf("\nPROGRAM:\n");
	cur = _vmem_map.first_region();
	while(cur) {
		printf("%x %x %d | %x\n", cur->start, cur->size, cur->used, cur->related ? cur->related->start : -1);
		cur = cur->next;
	}
	printf("\n");
}
