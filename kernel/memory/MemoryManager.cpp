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
#include <kernel/interrupt/interrupt.h>
#include "MemoryManager.h"
#include <kernel/multiboot.h>
#include "AnonymousVMObject.h"
#include <kernel/arch/i386/isr.h>
#include <kernel/device/DiskDevice.h>
#include <kernel/tasking/Thread.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/kstd/KLog.h>

size_t used_kheap_mem;

uint8_t early_kheap_memory[0x200000]; // 2MiB
size_t used_early_kheap_memory = 0;
bool did_setup_paging = false;

MemoryManager* MemoryManager::_inst;
Mutex MemoryManager::s_liballoc_lock {"liballoc"};
kstd::Arc<VMRegion> kernel_text_region;
kstd::Arc<VMRegion> kernel_data_region;
kstd::Arc<VMRegion> physical_pages_region;

MemoryManager::MemoryManager():
	m_kernel_space(kstd::Arc<VMSpace>::make(HIGHER_HALF, KERNEL_VIRTUAL_HEAP_BEGIN - HIGHER_HALF - PAGE_SIZE * 2, kernel_page_directory)),
	m_heap_space(kstd::Arc<VMSpace>::make(KERNEL_VIRTUAL_HEAP_BEGIN, ~0x0 - KERNEL_VIRTUAL_HEAP_BEGIN + 1 - PAGE_SIZE, kernel_page_directory))
{
	if(_inst)
		PANIC("MEMORY_MANAGER_DUPLICATE", "Something tried to initialize the memory manager twice.");
}

MemoryManager& MemoryManager::inst() {
	if(__builtin_expect(!_inst, false))
		_inst = new MemoryManager;
	return *_inst;
}

void MemoryManager::setup_paging() {
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
	KLog::dbg("Memory", "Mapping physical page array to pages {#x} -> {#x}", page_array_start_page,
			   page_array_start_page + page_array_num_pages - 1);

	// Setup and enable paging
	PageDirectory::init_paging();

	// Map the array to memory
	VMProt pages_prot = {
		.read = true,
		.write = true,
		.execute = false
	};
	for(size_t i = 0; i < page_array_num_pages; i++) {
		if(kernel_page_directory.map_page(kstd::ceil_div(KERNEL_DATA_END, PAGE_SIZE) + i, page_array_start_page + i, pages_prot).is_error())
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
		kernel_text_region = TRY(m_kernel_space->map_object(kernel_text_object, VMProt::RX, VirtualRange {KERNEL_TEXT, kernel_text_object->size()}));

		auto kernel_data_object = TRY(AnonymousVMObject::map_to_physical(KERNEL_DATA - HIGHER_HALF, KERNEL_DATA_SIZE));
		kernel_data_region = TRY(m_kernel_space->map_object(kernel_data_object, VMProt::RW, VirtualRange {KERNEL_DATA, kernel_data_object->size()}));

		auto physical_pages_object = TRY(AnonymousVMObject::map_to_physical(page_array_start_page * PAGE_SIZE, page_array_num_pages * PAGE_SIZE));
		physical_pages_region = TRY(m_kernel_space->map_object(physical_pages_object, VMProt::RW, VirtualRange { (VirtualAddress) m_physical_pages, physical_pages_object->size() }));

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
#if defined(__i386__)
	asm volatile("movl %0, %%cr3" :: "r"(page_directory.entries_physaddr()));
#endif
	// TODO: aarch64
}

void MemoryManager::page_fault_handler(ISRRegisters* regs) {
	TaskManager::ScopedCritical critical;
	uint32_t err_pos;
#if defined(__i386__)
	asm volatile ("mov %%cr2, %0" : "=r" (err_pos));
#endif
	// TODO: aarch64
	switch (regs->err_code) {
		case FAULT_KERNEL_READ:
			PANIC("KRNL_READ_NONPAGED_AREA", "0x%x", err_pos);
		case FAULT_KERNEL_READ_GPF:
			PANIC("KRNL_READ_PROTECTION_FAULT", "0x%x", err_pos);
		case FAULT_KERNEL_WRITE:
			PANIC("KRNL_WRITE_NONPAGED_AREA", "0x%x", err_pos);
		case FAULT_KERNEL_WRITE_GPF:
			PANIC("KRNL_WRITE_PROTECTION_FAULT", "0x%x", err_pos);
		case FAULT_USER_READ:
			PANIC("USR_READ_NONPAGED_AREA", "0x%x", err_pos);
		case FAULT_USER_READ_GPF:
			PANIC("USR_READ_PROTECTION_FAULT", "0x%x", err_pos);
		case FAULT_USER_WRITE:
			PANIC("USR_WRITE_NONPAGED_AREA", "0x%x", err_pos);
		case FAULT_USER_WRITE_GPF:
			PANIC("USR_WRITE_PROTECTION_FAULT", "0x%x", err_pos);
		default:
			PANIC("UNKNOWN_PAGE_FAULT", "0x%x", err_pos);
	}
}

void MemoryManager::invlpg(void* vaddr) {
#if defined(__i386__)
	asm volatile("invlpg %0" : : "m"(*(uint8_t*)vaddr) : "memory");
#endif
	// TODO: aarch64
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
		auto object = TRY(AnonymousVMObject::alloc(size, "kernel", false));
		return TRY(m_kernel_space->map_object(object, VMProt::RW));
	};
	auto res = do_alloc();
	if(res.is_error())
		PANIC("ALLOC_KERNEL_REGION_FAIL", "Could not allocate a new anonymous memory region for the kernel.");
	return res.value();
}

kstd::Arc<VMRegion> MemoryManager::alloc_kernel_stack_region(size_t size) {
	auto do_alloc = [&]() -> ResultRet<kstd::Arc<VMRegion>> {
		auto object = TRY(AnonymousVMObject::alloc(size, "kernel_stack", true));
		return TRY(m_kernel_space->map_object_with_sentinel(object, VMProt::RW));
	};
	auto res = do_alloc();
	if(res.is_error())
		PANIC("ALLOC_KERNEL_REGION_FAIL", "Could not allocate a new anonymous memory region for the kernel.");
	return res.value();
}

kstd::Arc<VMRegion> MemoryManager::alloc_dma_region(size_t size) {
	auto do_alloc = [&]() -> ResultRet<kstd::Arc<VMRegion>> {
		auto object = TRY(AnonymousVMObject::alloc_contiguous(size, "dma"));
		return TRY(m_kernel_space->map_object(object, VMProt::RW));
	};
	auto res = do_alloc();
	if(res.is_error())
		PANIC("ALLOC_DMA_REGION_FAIL", "Could not allocate a new anonymous memory region for DMA.");
	return res.value();
}

kstd::Arc<VMRegion> MemoryManager::alloc_contiguous_kernel_region(size_t size) {
	auto do_alloc = [&]() -> ResultRet<kstd::Arc<VMRegion>> {
		auto object = TRY(AnonymousVMObject::alloc_contiguous(size, "kernel_contig"));
		return TRY(m_kernel_space->map_object(object, VMProt::RW));
	};
	auto res = do_alloc();
	if(res.is_error())
		PANIC("ALLOC_DMA_REGION_FAIL", "Could not allocate a new contiguous anonymous memory region.");
	return res.value();
}

kstd::Arc<VMRegion> MemoryManager::alloc_mapped_region(PhysicalAddress start, size_t size) {
	auto do_map = [&]() -> ResultRet<kstd::Arc<VMRegion>> {
		auto object = TRY(AnonymousVMObject::map_to_physical(start, size));
		return TRY(m_kernel_space->map_object(object, VMProt::RW));
	};
	auto res = do_map();
	if(res.is_error())
		PANIC("ALLOC_MAPPED_FAIL", "Could not map a physical region into kernel space.");
	return res.value();
}

kstd::Arc<VMRegion> MemoryManager::map_object(kstd::Arc<VMObject> object, VirtualRange range) {
	auto res = m_kernel_space->map_object(object, VMProt::RW, {0, range.size}, range.start);
	if(res.is_error())
		PANIC("ALLOC_MAPPED_FAIL", "Could not map an existing object into kernel space.");
	return res.value();
}

void MemoryManager::copy_page(PageIndex src, PageIndex dest) {
	MM.with_dual_quickmapped(src, dest, [](void* src_ptr, void* dest_ptr) {
		memcpy_uint32((uint32_t*) dest_ptr, (uint32_t*) src_ptr, PAGE_SIZE / sizeof(uint32_t));
	});
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
	// Get some physical pages
	if(num_pages > 4906)
		PANIC("KHEAP_ALLOC_TOO_BIG", "Tried allocating more than 4096 pages at once for the kernel heap.");

	for(size_t i = 0; i < num_pages; i++)
		m_heap_pages[i] = TRY(alloc_physical_page());
	m_num_heap_pages = num_pages;

	// Find a free area in the heap space. We don't allocate it yet, as that would call `kmalloc` :)
	m_last_heap_loc = TRY(m_heap_space->find_free_space(num_pages * PAGE_SIZE));

	// Map the pages into kernel space.
	size_t start_vpage = m_last_heap_loc / PAGE_SIZE;
	for(size_t i = 0; i < num_pages; i++) {
		kernel_page_directory.map_page(start_vpage + i, m_heap_pages[i], VMProt{
			.read = true,
			.write = true,
			.execute = false
		});
	}

	// Zero them out and return the address.
	memset((void*) m_last_heap_loc, 0, num_pages * PAGE_SIZE);
	return m_last_heap_loc;
}

void MemoryManager::finalize_heap_pages() {
	if(!did_setup_paging || !m_num_heap_pages)
		return;

	static bool finalizing_heap = false;
	if(finalizing_heap)
		return;
	finalizing_heap = true;

	// Now, officially map the pages in the "correct" way.
	m_heap_pages.resize(m_num_heap_pages);
	auto object = kstd::Arc<VMObject>(new VMObject("kheap", m_heap_pages));
	auto res = m_heap_space->map_object(object, VMProt::RW, VirtualRange {m_last_heap_loc, object->size() });
	if(res.is_error())
		PANIC("KHEAP_FINALIZE_FAIL", "We weren't able to map the new heap region where the VMSpace said we could.");

	// Leak a reference to the region since we'll manually un-leak it later
	res.value().leak_ref();

	// Unlock the heap and reset everything
	m_num_heap_pages = 0;
	m_heap_pages.resize(4096);
	finalizing_heap = false;
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
	MemoryManager::s_liballoc_lock.acquire();
}

void liballoc_unlock() {
	MemoryManager::s_liballoc_lock.release();
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
	if (did_setup_paging)
		MM.finalize_heap_pages();
}

void liballoc_free(void *ptr, int pages) {
	used_kheap_mem -= pages * PAGE_SIZE;

	if(ptr > early_kheap_memory && ptr < early_kheap_memory + sizeof(early_kheap_memory)) {
//		KLog::dbg("Memory", "Tried freeing early kheap memory! This doesn't do anything.");
		return;
	}

	// Find the region we need to free and delete it, since we manually leaked its reference count
	auto region_res = MM.heap_space()->get_region_at((VirtualAddress) ptr);
	if(region_res.is_error())
		PANIC("LIBALLOC_FREE_FAIL", "Could not find the VMRegion associated with a call to liballoc_free.");
	region_res.value().leak_unref();
}
