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

#include <kernel/kstd/kstddef.h>
#include <kernel/kstd/kstdio.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/interrupt/interrupt.h>
#include "PageTable.h"
#include "MemoryManager.h"
#include <kernel/multiboot.h>
#include "AnonymousVMObject.h"
#include <kernel/device/DiskDevice.h>
#include <kernel/tasking/Thread.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/kstd/KLog.h>

size_t usable_bytes_ram = 0;
size_t total_bytes_ram = 0;
size_t reserved_bytes_ram = 0;
size_t bad_bytes_ram = 0;
size_t usable_lower_limt = ~0;
size_t usable_upper_limit = 0;
size_t mem_lower_limit = ~0;
size_t mem_upper_limit = 0;
size_t used_kheap_mem;

uint8_t early_kheap_memory[0x200000]; // 2MiB
size_t used_early_kheap_memory = 0;
bool did_setup_paging = false;

MemoryManager* MemoryManager::_inst;
kstd::Arc<VMRegion> kernel_text_region;
kstd::Arc<VMRegion> kernel_data_region;
kstd::Arc<VMRegion> physical_pages_region;

MemoryManager::MemoryManager():
	m_kernel_space(kstd::make_shared<VMSpace>(HIGHER_HALF, ~0x0 - HIGHER_HALF + 1 - PAGE_SIZE, kernel_page_directory))
{
	if(_inst)
		PANIC("MEMORY_MANAGER_DUPLICATE", "Something tried to initialize the memory manager twice.");
	_inst = this;
}

MemoryManager& MemoryManager::inst() {
	return *_inst;
}

void MemoryManager::setup_paging() {
	// Setup and enable paging
	PageDirectory::init_paging();

	// Find a region where we can store our physical page array
	size_t num_physical_pages = mem_upper_limit / PAGE_SIZE;
	size_t page_array_num_pages = kstd::ceil_div(num_physical_pages * sizeof(PhysicalPage), PAGE_SIZE);
	size_t page_array_start_page = 0;
	for(size_t i = 0; i < m_physical_regions.size(); i++) {
		auto& region = m_physical_regions[i];
		if(region->reserved() || region->num_pages() != region->free_pages() || region->free_pages() < page_array_num_pages)
			continue;

		page_array_start_page = m_physical_regions[i]->start_page();

		// Resize the physical region so that we don't allocate these pages
		size_t new_size = region->num_pages() - page_array_num_pages;
		size_t new_start = region->start_page() + page_array_num_pages;
		delete region;
		if(new_size)
			m_physical_regions[i] = new PhysicalRegion(new_start, new_size, false, false);
		else
			m_physical_regions.erase(i);

		break;
	}

	// Make sure we found a page
	if(page_array_start_page == 0)
		PANIC("PAGE_ARRAY_NOMEM", "Cannot find enough contiguous memory to store the physical page array.");
	KLog::dbg("Memory", "Mapping physical page array to pages 0x%x -> 0x%x", page_array_start_page, page_array_start_page + page_array_num_pages - 1);

	// Map the array to memory
	VMProt pages_prot = {
		.read = true,
		.write = true,
		.execute = false,
		.cow = false
	};
	for(size_t i = 0; i < page_array_num_pages; i++) {
		if(kernel_page_directory.map_page(kstd::ceil_div(KERNEL_DATA_END, PAGE_SIZE) + i, page_array_start_page, pages_prot).is_error())
			PANIC("PAGE_ARRAY_MAP_ERR", "Could not map the physical page array.");
	}

	// Set the pointer to the physical pages array and zero it out
	m_physical_pages = (PhysicalPage*) (kstd::ceil_div(KERNEL_DATA_END, PAGE_SIZE) * PAGE_SIZE);
	memset(m_physical_pages, 0, num_physical_pages * sizeof(PhysicalPage));

	// Setup the physical region freelists
	for(size_t i = 0; i < m_physical_regions.size(); i++)
		m_physical_regions[i]->init();

	// Now that we're all set up to use normal methods of mapping stuff, map the kernel and physical pages again
	auto do_map = [&]() -> Result {
		auto kernel_text_object = TRY(AnonymousVMObject::map_to_physical(KERNEL_TEXT - HIGHER_HALF, KERNEL_TEXT_SIZE));
		kernel_text_region = TRY(m_kernel_space->map_object(kernel_text_object, KERNEL_TEXT, VMProt::RX));

		auto kernel_data_object = TRY(AnonymousVMObject::map_to_physical(KERNEL_DATA - HIGHER_HALF, KERNEL_DATA_SIZE));
		kernel_data_region = TRY(m_kernel_space->map_object(kernel_data_object, KERNEL_DATA, VMProt::RW));

		auto physical_pages_object = TRY(AnonymousVMObject::map_to_physical(page_array_start_page * PAGE_SIZE, page_array_num_pages * PAGE_SIZE));
		physical_pages_region = TRY(m_kernel_space->map_object(physical_pages_object, (VirtualAddress) m_physical_pages, VMProt::RW));

		return Result(SUCCESS);
	};

	// Make sure the kernel is reserved in the memory space
	auto map_res = do_map();
	ASSERT(map_res.is_success());

	did_setup_paging = true;
}

void MemoryManager::load_page_directory(const kstd::Arc<PageDirectory>& page_directory) {
	load_page_directory(*page_directory);
}

void MemoryManager::load_page_directory(PageDirectory* page_directory) {
	load_page_directory(*page_directory);
}

void MemoryManager::load_page_directory(PageDirectory& page_directory) {
	asm volatile("movl %0, %%cr3" :: "r"(page_directory.entries_physaddr()));
}

void MemoryManager::page_fault_handler(struct Registers *r) {
	Interrupt::Disabler disabler;
	uint32_t err_pos;
	asm volatile ("mov %%cr2, %0" : "=r" (err_pos));
	switch (r->err_code) {
		case 0:
			PANIC("KRNL_READ_NONPAGED_AREA", "0x%x", err_pos);
		case 1:
			PANIC("KRNL_READ_PROTECTION_FAULT", "0x%x", err_pos);
		case 2:
			PANIC("KRNL_WRITE_NONPAGED_AREA", "0x%x", err_pos);
		case 3:
			PANIC("KRNL_WRITE_PROTECTION_FAULT", "0x%x", err_pos);
		case 4:
			PANIC("USR_READ_NONPAGED_AREA", "0x%x", err_pos);
		case 5:
			PANIC("USR_READ_PROTECTION_FAULT", "0x%x", err_pos);
		case 6:
			PANIC("USR_WRITE_NONPAGED_AREA", "0x%x", err_pos);
		case 7:
			PANIC("USR_WRITE_PROTECTION_FAULT", "0x%x", err_pos);
		default:
			PANIC("UNKNOWN_PAGE_FAULT", "0x%x", err_pos);
	}
}

void MemoryManager::invlpg(void* vaddr) {
	asm volatile("invlpg %0" : : "m"(*(uint8_t*)vaddr) : "memory");
}

void MemoryManager::parse_mboot_memory_map(struct multiboot_info* header, struct multiboot_mmap_entry* mmap_entry) {
	size_t mmap_offset = 0;
	usable_bytes_ram = 0;

	auto make_region = [&](size_t addr, size_t size, bool reserved, bool used) {
		//Round up the address of the entry to a page boundary and round the size down to a page boundary
		uint32_t addr_pagealigned = ((addr + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
		uint32_t size_pagealigned = ((size - (addr_pagealigned - addr)) / PAGE_SIZE) * PAGE_SIZE;

		// We don't want the zero page.
		if(addr_pagealigned == 0) {
			addr_pagealigned += PAGE_SIZE;
			if(size_pagealigned)
				size_pagealigned -= PAGE_SIZE;
		}

		if(size_pagealigned / PAGE_SIZE < 2) {
			KLog::dbg("Memory", "Ignoring too-small memory region at 0x%x", addr);
			return;
		}

		KLog::dbg("Memory", "Making memory region at page 0x%x of 0x%x pages (%s, %s)", addr_pagealigned / PAGE_SIZE, size_pagealigned / PAGE_SIZE, used ? "Used" : "Unused", reserved ? "Reserved" : "Unreserved");
		auto region = new PhysicalRegion(
			addr_pagealigned / PAGE_SIZE,
			size_pagealigned / PAGE_SIZE,
			reserved,
			used
		);
		total_bytes_ram += size_pagealigned;

		if(!region->reserved() && region->free_pages()) {
			if(addr_pagealigned < usable_lower_limt)
				usable_lower_limt = addr_pagealigned;
			if(addr_pagealigned + (size_pagealigned - 1) > usable_upper_limit)
				usable_upper_limit = addr_pagealigned + (size_pagealigned - 1);
		}

		if(addr_pagealigned < mem_lower_limit)
			mem_lower_limit = addr_pagealigned;
		if(addr_pagealigned + (size_pagealigned - 1) > mem_upper_limit)
			mem_upper_limit = addr_pagealigned + (size_pagealigned - 1);

		if(!region->reserved())
			usable_bytes_ram += size_pagealigned;
		else
			reserved_bytes_ram += size_pagealigned;

		if(mmap_entry->type == MULTIBOOT_MEMORY_BADRAM)
			bad_bytes_ram += size_pagealigned;

		m_physical_regions.push_back(region);

		KLog::dbg("Memory", "Adding memory region at page 0x%x of 0x%x pages (%s, %s)", region->start_page(), region->num_pages(), !region->free_pages() ? "Used" : "Unused", region->reserved() ? "Reserved" : "Unreserved");
	};

	while(mmap_offset < header->mmap_length) {
		if(mmap_entry->addr_high || mmap_entry->len_high) {
			//If the entry is in extended memory, ignore it
			KLog::dbg("Memory", "Ignoring memory region above 4GiB (0x%x%x)",
					mmap_entry->addr_high, mmap_entry->addr_low);
		} else {
			size_t addr = mmap_entry->addr_low;
			size_t size = mmap_entry->len_low;
			size_t end = addr + size;

			// Check if the kernel is contained inside of this region. If so, we need to bifurcate it.
			if(KERNEL_DATA_END - HIGHER_HALF >= addr && KERNEL_DATA_END - HIGHER_HALF <= end) {
				KLog::dbg("Memory", "Kernel is from pages 0x%x --> 0x%x", (KERNEL_TEXT - HIGHER_HALF) / PAGE_SIZE, (KERNEL_DATA_END - HIGHER_HALF) / PAGE_SIZE - 1);
				if(addr < KERNEL_TEXT - HIGHER_HALF) {
					// Space in region before kernel
					make_region(addr,
								KERNEL_TEXT - addr,
								mmap_entry->type == MULTIBOOT_MEMORY_RESERVED,
								mmap_entry->type != MULTIBOOT_MEMORY_AVAILABLE);
				}
				if(end > KERNEL_DATA_END - HIGHER_HALF) {
					// Space in region after kernel
					make_region(KERNEL_DATA_END - HIGHER_HALF,
								end - (KERNEL_DATA_END - HIGHER_HALF),
								mmap_entry->type == MULTIBOOT_MEMORY_RESERVED,
								mmap_entry->type != MULTIBOOT_MEMORY_AVAILABLE);
				}
			} else {
				make_region(addr,
							size,
							mmap_entry->type == MULTIBOOT_MEMORY_RESERVED,
							mmap_entry->type != MULTIBOOT_MEMORY_AVAILABLE);
			}
		}
		mmap_offset += mmap_entry->size + sizeof(mmap_entry->size);
		mmap_entry = (struct multiboot_mmap_entry*) ((size_t)mmap_entry + mmap_entry->size + sizeof(mmap_entry->size));
	}

	KLog::dbg("Memory", "Usable memory limits: 0x%x -> 0x%x", usable_lower_limt, usable_upper_limit);
	KLog::dbg("Memory", "Total memory limits: 0x%x -> 0x%x", mem_lower_limit, mem_upper_limit);
}

ResultRet<PageIndex> MemoryManager::alloc_physical_page() const {
	for(size_t i = 0; i < m_physical_regions.size(); i++) {
		auto result = m_physical_regions[i]->alloc_page();
		if(!result.is_error()) {
			PageIndex ret = result.value();
			// Set the refcount of the page to 1
			auto& page = get_physical_page(ret);
			page.allocated.ref_count = 1;
			page.allocated.reserved = false;
			return ret;
		}
	}

	// We couldn't allocate any physical pages. Try freeing four for good measure.
	if(DiskDevice::free_pages(4) >= 1)
		return alloc_physical_page();

	// No more pages. This is bad.
	PANIC("NO_MEM", "The system ran out of physical memory.");
}

ResultRet<kstd::vector<PageIndex>> MemoryManager::alloc_physical_pages(size_t num_pages) const {
	// If we already know we won't have enough free memory, try freeing twice as many up in the disk cache first
	if((usable_bytes_ram - used_pmem()) / PAGE_SIZE < num_pages)
		DiskDevice::free_pages(num_pages * 2);

	auto new_pages = kstd::vector<PageIndex>();
	new_pages.reserve(num_pages);
	while(num_pages--)
		new_pages.push_back(TRY(MemoryManager::inst().alloc_physical_page()));
	return new_pages;
}

ResultRet<kstd::vector<PageIndex>> MemoryManager::alloc_contiguous_physical_pages(size_t num_pages) const {
	for(size_t i = 0; i < m_physical_regions.size(); i++) {
		auto result = m_physical_regions[i]->alloc_pages(num_pages);
		if(!result.is_error()) {
			PageIndex first_page = result.value();
			kstd::vector<PageIndex> ret;
			ret.reserve(num_pages);
			for(size_t page_index = 0; page_index < num_pages; page_index++) {
				auto& page = get_physical_page(first_page + page_index);
				page.allocated.ref_count = 1;
				page.allocated.reserved = false;
				ret.push_back(first_page + page_index);
			}
			return ret;
		}
	}

	return Result(ENOMEM);
}

kstd::Arc<VMRegion> MemoryManager::alloc_kernel_region(size_t size) {
	auto do_alloc = [&]() -> ResultRet<kstd::Arc<VMRegion>> {
		auto object = TRY(AnonymousVMObject::alloc(size));
		return TRY(m_kernel_space->map_object(object));
	};
	auto res = do_alloc();
	if(res.is_error())
		PANIC("ALLOC_KERNEL_REGION_FAIL", "Could not allocate a new anonymous memory region for the kernel.");
	return res.value();
}

kstd::Arc<VMRegion> MemoryManager::alloc_dma_region(size_t size) {
	auto do_alloc = [&]() -> ResultRet<kstd::Arc<VMRegion>> {
		auto object = TRY(AnonymousVMObject::alloc_contiguous(size));
		return TRY(m_kernel_space->map_object(object));
	};
	auto res = do_alloc();
	if(res.is_error())
		PANIC("ALLOC_DMA_REGION_FAIL", "Could not allocate a new anonymous memory region for DMA.");
	return res.value();
}

kstd::Arc<VMRegion> MemoryManager::alloc_mapped_region(PhysicalAddress start, size_t size) {
	auto do_map = [&]() -> ResultRet<kstd::Arc<VMRegion>> {
		auto object = TRY(AnonymousVMObject::map_to_physical(start, size));
		return TRY(m_kernel_space->map_object(object));
	};
	auto res = do_map();
	if(res.is_error())
		PANIC("ALLOC_MAPPED_FAIL", "Could not map a physical region into kernel space.");
	return res.value();
}

kstd::Arc<VMRegion> MemoryManager::map_object(kstd::Arc<VMObject> object) {
	auto res = m_kernel_space->map_object(object);
	if(res.is_error())
		PANIC("ALLOC_MAPPED_FAIL", "Could not map an existing object into kernel space.");
	return res.value();
}

void MemoryManager::free_physical_page(PageIndex page) const {
	ASSERT(get_physical_page(page).allocated.ref_count.load(MemoryOrder::Relaxed) == 0);

	for(size_t i = 0; i < m_physical_regions.size(); i++) {
		if(m_physical_regions[i]->contains_page(page)) {
			m_physical_regions[i]->free_page(page);
			return;
		}
	}

	ASSERT(false);
}

ResultRet<VirtualAddress> MemoryManager::alloc_heap_pages(size_t num_pages) {
	LOCK(m_heap_lock);
	ASSERT(m_num_heap_pages == 0); // Make sure this isn't already in progress... Just in case.

	// Get some physical pages
	if(num_pages > 1024)
		PANIC("KHEAP_ALLOC_TOO_BIG", "Tried allocating more than 1024 pages at once for the kernel heap.");

	for(size_t i = 0; i < num_pages; i++)
		m_heap_pages[i] = TRY(alloc_physical_page());
	m_num_heap_pages = num_pages;

	// Lock the kernel vmem space so we don't mess with regions in the meantime, and then find some free space.
	m_kernel_space->lock().acquire();
	m_last_heap_loc = TRY(m_kernel_space->find_free_space(num_pages * PAGE_SIZE));

	// Map the pages into kernel space.
	size_t start_vpage = m_last_heap_loc / PAGE_SIZE;
	for(size_t i = 0; i < num_pages; i++) {
		kernel_page_directory.map_page(start_vpage + i, m_heap_pages[i], VMProt{
			.read = true,
			.write = true,
			.execute = false,
			.cow = false
		});
	}

	// Zero them out and return the address.
	memset((void*) m_last_heap_loc, 0, num_pages * PAGE_SIZE);
	return m_last_heap_loc;
}

void MemoryManager::finalize_heap_pages() {
	if(!did_setup_paging)
		return;

	LOCK(m_heap_lock);
	if(!m_num_heap_pages)
		return; // There's nothing to do.

	static bool is_finalizing = false;
	if(is_finalizing)
		return;
	is_finalizing = true;

	m_heap_pages.resize(m_num_heap_pages);

	// Now, officially map the pages in the "correct" way.
	auto object = kstd::Arc<VMObject>(new VMObject(m_heap_pages));
	auto res = m_kernel_space->map_object(object, m_last_heap_loc);
	if(res.is_error())
		PANIC("KHEAP_FINALIZE_FAIL", "We weren't able to map the new heap region where the VMSpace said we could.");

	// Leak a reference to the region since we'll manually un-leak it later
	res.value().leak_ref();

	// Unlock the vmem space and reset everything
	is_finalizing = false;
	m_kernel_space->lock().release();
	m_num_heap_pages = 0;
	m_heap_pages.resize(1024);
}

size_t MemoryManager::usable_mem() const {
	return usable_bytes_ram;
}

size_t MemoryManager::used_pmem() const {
	size_t used_pages = 0;
	for(size_t i = 0; i < m_physical_regions.size(); i++)
		used_pages += m_physical_regions[i]->num_pages() - m_physical_regions[i]->free_pages();
	return used_pages * PAGE_SIZE;
}

size_t MemoryManager::reserved_pmem() const {
	return reserved_bytes_ram;
}

size_t MemoryManager::kernel_vmem() const {
	return m_kernel_space->used();
}

size_t MemoryManager::kernel_pmem() const {
	return kernel_vmem(); //TODO: A better way of tracking who's using how much physical memory...
}

size_t MemoryManager::kernel_heap() const {
	return used_kheap_mem;
}

void liballoc_lock() {
	MemoryManager::inst().liballoc_spinlock.acquire();
}

void liballoc_unlock() {
	MemoryManager::inst().liballoc_spinlock.release();
}

void *liballoc_alloc(int pages) {
	used_kheap_mem += pages * PAGE_SIZE;

	// If we still have early kheap memory, use it
	if(pages * PAGE_SIZE < (sizeof(early_kheap_memory) - used_early_kheap_memory)) {
		void* ptr = early_kheap_memory + used_early_kheap_memory;
		used_early_kheap_memory += pages * PAGE_SIZE;
		memset(ptr, 0, pages * PAGE_SIZE);
		return ptr;
	}

	ASSERT(did_setup_paging);
	auto alloc_res = MM.alloc_heap_pages(pages);
	if(alloc_res.is_error())
		PANIC("KHEAP_ALLOC_FAIL", "We couldn't allocate a new region for the kernel heap.");

	return (void*) alloc_res.value();
}

void liballoc_afteralloc(void* ptr_alloced) {
	MM.finalize_heap_pages();
}

void liballoc_free(void *ptr, int pages) {
	used_kheap_mem -= pages * PAGE_SIZE;

	if(ptr > early_kheap_memory && ptr < early_kheap_memory + sizeof(early_kheap_memory)) {
//		KLog::dbg("Memory", "Tried freeing early kheap memory! This doesn't do anything.");
		return;
	}

	// Find the region we need to free and delete it, since we manually leaked its reference count
	auto region_res = MM.kernel_space()->get_region_at((VirtualAddress) ptr);
	if(region_res.is_error())
		PANIC("LIBALLOC_FREE_FAIL", "Could not find the VMRegion associated with a call to liballoc_free.");
	region_res.value().leak_unref();
}
