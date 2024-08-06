/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include <kernel/memory/MemoryManager.h>
#include <kernel/kstd/KLog.h>
#include <kernel/multiboot.h>

extern multiboot_info mboot_header;
extern multiboot_mmap_entry* mmap_entry;

void MemoryManager::setup_device_memory_map() {
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
			KLog::dbg("Memory", "Ignoring too-small memory region at {#x}", addr);
			return;
		}

		KLog::dbg("Memory", "Making memory region at page {#x} of {#x} pages ({}, {})", addr_pagealigned / PAGE_SIZE,
				  size_pagealigned / PAGE_SIZE, used ? "Used" : "Unused", reserved ? "Reserved" : "Unreserved");
		auto region = new PhysicalRegion(
				addr_pagealigned / PAGE_SIZE,
				size_pagealigned / PAGE_SIZE,
				reserved,
				used
		);
		total_bytes_ram += size_pagealigned;

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

		KLog::dbg("Memory", "Adding memory region at page {#x} of {#x} pages ({}, {})", region->start_page(),
				  region->num_pages(), !region->free_pages() ? "Used" : "Unused",
				  region->reserved() ? "Reserved" : "Unreserved");
	};

	while(mmap_offset < mboot_header.mmap_length) {
		if(mmap_entry->addr_high || mmap_entry->len_high) {
			//If the entry is in extended memory, ignore it
			KLog::dbg("Memory", "Ignoring memory region above 4GiB (0x{x}{x})",
					  mmap_entry->addr_high, mmap_entry->addr_low);
		} else {
			size_t addr = mmap_entry->addr_low;
			size_t size = mmap_entry->len_low;
			size_t end = addr + size;

			// Check if the kernel is contained inside of this region. If so, we need to bifurcate it.
			if(KERNEL_DATA_END - HIGHER_HALF >= addr && KERNEL_DATA_END - HIGHER_HALF <= end) {
				KLog::dbg("Memory", "Kernel is from pages {#x} --> {#x}", (KERNEL_TEXT - HIGHER_HALF) / PAGE_SIZE,
						  (KERNEL_DATA_END - HIGHER_HALF) / PAGE_SIZE - 1);
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

	KLog::dbg("Memory", "Total memory limits: {#x} -> {#x}", mem_lower_limit, mem_upper_limit);
}