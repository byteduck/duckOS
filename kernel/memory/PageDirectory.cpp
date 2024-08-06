/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "PageDirectory.h"
#include "MemoryManager.h"
#include <kernel/kstd/KLog.h>

void PageDirectory::map(VMRegion& region, VirtualRange range) {
	LOCK(m_lock);

	if(range.size == 0)
		range.size = region.size();

	PageIndex start_vpage = region.start() / PAGE_SIZE;
	PageIndex start_index = range.start / PAGE_SIZE;
	PageIndex end_index = (range.start + range.size) / PAGE_SIZE;
	PageIndex page_offset = region.object_start() / PAGE_SIZE;
	auto prot = region.prot();
	auto is_device = region.object()->is_device();
	ASSERT(prot.read);
	ASSERT(range.start % PAGE_SIZE == 0);
	ASSERT(range.size % PAGE_SIZE == 0);
	ASSERT(range.start + range.size <= region.end());

	for(size_t page_index = start_index; page_index < end_index; page_index++) {
		auto& page = region.object()->physical_page(page_index + page_offset);
		if(!page.index())
			continue;

		auto vpage = start_vpage + page_index;
		auto ppage = region.object()->physical_page(page_index + page_offset).index();
		VMProt page_prot = {
			.read = prot.read,
			.write = region.object()->page_is_cow(page_index) ? false : prot.write,
			.execute = prot.execute
		};

#if defined(__i386__)
		if(map_page(vpage, ppage, page_prot).is_error())
			return;
#elif defined(__aarch64__)
		if(map_page(vpage, ppage, page_prot, is_device).is_error())
			return;
#endif
	}
}

void PageDirectory::unmap(VMRegion& region, VirtualRange range) {
	LOCK(m_lock);

	if(range.size == 0)
		range.size = region.size();

	PageIndex start_vpage = region.start() / PAGE_SIZE;
	PageIndex start_index = range.start / PAGE_SIZE;
	PageIndex end_index = (range.start + range.size) / PAGE_SIZE;

	ASSERT(range.start % PAGE_SIZE == 0);
	ASSERT(range.size % PAGE_SIZE == 0);
	ASSERT(range.start + range.size <= region.end());

	if(region.end() > HIGHER_HALF && m_type != DirectoryType::KERNEL) {
		KLog::warn("PageDirectory", "Tried unmapping kernel in non-kernel directory!");
		return;
	}

	for(size_t page_index = start_index; page_index < end_index; page_index++) {
		if(unmap_page(start_vpage + page_index).is_error())
			return;
	}
}

void PageDirectory::setup_kernel_map() {
	// Manually map pages for kernel to start with.
	auto map_range = [&](VirtualAddress vstart, VirtualAddress pstart, size_t size, VMProt prot) {
		size_t start_vpage = vstart / PAGE_SIZE;
		size_t start_ppage = pstart / PAGE_SIZE;
		size_t num_pages = ((vstart + size) / PAGE_SIZE) - start_vpage;
		for(size_t i = 0; i < num_pages; i++)
			if(MM.kernel_page_directory.map_page(start_vpage + i, start_ppage + i, prot).is_error())
				PANIC("PAGING_INIT_FAIL", "Could not map the kernel when setting up paging.");
	};

	// Map the kernel text and data
	map_range(KERNEL_TEXT, KERNEL_TEXT - HIGHER_HALF, KERNEL_TEXT_SIZE, VMProt::RX);
	map_range(KERNEL_DATA, KERNEL_DATA - HIGHER_HALF, KERNEL_DATA_SIZE, VMProt::RW);
}

size_t PageDirectory::get_physaddr(void *virtaddr) {
	return get_physaddr((size_t)virtaddr);
}