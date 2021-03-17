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

#include <kernel/kstd/kstddef.h>
#include <kernel/kstd/kstdio.h>
#include <kernel/memory/Memory.h>
#include <kernel/interrupt/isr.h>
#include <kernel/memory/PageDirectory.h>
#include "PageTable.h"
#include "MemoryMap.h"

namespace Memory {
	PageDirectory kernel_page_directory;
	PageDirectory::Entry kernel_page_directory_entries[1024] __attribute__((aligned(4096)));
	PageTable::Entry kernel_early_page_table_entries1[1024] __attribute__((aligned(4096)));
	PageTable::Entry kernel_early_page_table_entries2[1024] __attribute__((aligned(4096)));

	MemoryMap _pmem_map(0,nullptr);
	MemoryRegion kernel_pmem_region = {0,0};
	MemoryRegion multiboot_memory_regions[32];
	MemoryRegion early_pmem_text_region_storage[2];
	MemoryRegion early_pmem_data_region_storage[2];
	uint8_t num_multiboot_memory_regions = 0;

	size_t usable_bytes_ram = 0;
	size_t total_bytes_ram = 0;
	size_t reserved_bytes_ram = 0;
	size_t bad_bytes_ram = 0;

	//TODO: Assumes computer has at least 4GiB of memory. Should detect memory in future.
	void setup_paging() {
		//Assert that the kernel doesn't exceed 7MiB
		ASSERT(KERNEL_DATA_END - KERNEL_TEXT <= 0x700000);

		kernel_page_directory.set_entries(kernel_page_directory_entries);
		PageDirectory::init_kmem();

		//Setup kernel page directory to map the kernel to HIGHER_HALF
		PageTable kernel_early_page_table1(0, nullptr, false);
		kernel_early_page_table1.entries() = kernel_early_page_table_entries1;
		early_pagetable_setup(&kernel_early_page_table1, HIGHER_HALF, true);
		for (auto i = 0; i < 1024; i++) {
			kernel_early_page_table1[i].data.present = true;
			kernel_early_page_table1[i].data.read_write = true;
			kernel_early_page_table1[i].data.set_address(PAGE_SIZE * i);
		}

		PageTable kernel_early_page_table2(0, nullptr, false);
		kernel_early_page_table2.entries() = kernel_early_page_table_entries2;
		early_pagetable_setup(&kernel_early_page_table2, HIGHER_HALF + PAGE_SIZE * 1024, true);
		for (auto i = 0; i < 1024; i++) {
			kernel_early_page_table2[i].data.present = true;
			kernel_early_page_table2[i].data.read_write = true;
			kernel_early_page_table2[i].data.set_address(PAGE_SIZE * i + PAGE_SIZE * 1024);
		}

		//Enable paging
		asm volatile(
				"movl %%eax, %%cr3\n" //Put the page directory pointer in cr3
				"movl %%cr0, %%eax\n"
				"orl $0x80000000, %%eax\n" //Set the proper flags in cr0
				"movl %%eax, %%cr0\n"
				: : "a"((size_t) kernel_page_directory.entries() - HIGHER_HALF)
		);

		//Setup the pmem map
		_pmem_map = MemoryMap(PAGE_SIZE, &multiboot_memory_regions[0]);
		MemoryRegion* text_region = _pmem_map.allocate_region(KERNEL_TEXT - HIGHER_HALF, KERNEL_TEXT_SIZE, early_pmem_text_region_storage);
		if(!text_region)
			PANIC("KRNL_MAP_FAIL", "The kernel's text section could not be allocated in the physical memory map.\n", true);
		MemoryRegion* data_region = _pmem_map.allocate_region(KERNEL_DATA - HIGHER_HALF, KERNEL_DATA_SIZE, early_pmem_data_region_storage);
		if(!text_region)
			PANIC("KRNL_MAP_FAIL", "The kernel's data section could not be allocated in the physical memory map.\n", true);
		_pmem_map.recalculate_memory_totals();

		//Now, map and write everything to the directory
		PageDirectory::map_kernel(text_region, data_region);
		kernel_page_directory.update_kernel_entries();
	}


	MemoryMap& pmem_map() {
		return _pmem_map;
	}


	void page_fault_handler(struct Registers *r) {
		cli();
		switch (r->err_code) {
			case 0:
				PANIC("KRNL_READ_NONPAGED_AREA", "", false);
				break;
			case 1:
				PANIC("KRNL_READ_PROTECTION_FAULT", "", false);
				break;
			case 2:
				PANIC("KRNL_WRITE_NONPAGED_AREA", "", false);
				break;
			case 3:
				PANIC("KRNL_WRITE_PROTECTION_FAULT", "", false);
				break;
			case 4:
				PANIC("USR_READ_NONPAGED_AREA", "", false);
				break;
			case 5:
				PANIC("USR_READ_PROTECTION_FAULT", "", false);
				break;
			case 6:
				PANIC("USR_WRITE_NONPAGED_AREA", "", false);
				break;
			case 7:
				PANIC("USR_WRITE_PROTECTION_FAULT", "", false);
				break;
			default:
				PANIC("UNKNOWN_PAGE_FAULT", "", false);
				break;
		}

		uint32_t err_pos;
		asm volatile ("mov %%cr2, %0" : "=r" (err_pos));
		printf("Virtual address: 0x%X\n", err_pos);
		print_regs(r);
		while (true);
	}


	size_t get_used_mem() {
		return pmem_map().used_memory();
	}

	size_t get_reserved_mem() {
		return pmem_map().reserved_memory();
	}

	size_t get_usable_mem() {
		return usable_bytes_ram;
	}

	size_t get_kernel_vmem() {
		return PageDirectory::kernel_vmem_map.used_memory();
	}

	size_t get_kernel_pmem() {
		return PageDirectory::used_kernel_pmem;
	}

	size_t get_kheap_pmem() {
		return PageDirectory::used_kheap_pmem;
	}

	void early_pagetable_setup(PageTable *page_table, size_t virtual_address, bool read_write) {
		ASSERT(virtual_address % PAGE_SIZE == 0);

		size_t index = virtual_address / PAGE_SIZE;
		size_t dir_index = (index / 1024) % 1024;

		kernel_page_directory[dir_index].data.present = true;
		kernel_page_directory[dir_index].data.read_write = read_write;
		kernel_page_directory[dir_index].data.size = PAGE_SIZE_FLAG;
		kernel_page_directory[dir_index].data.set_address((size_t) page_table->entries() - HIGHER_HALF);
	}

	void invlpg(void* vaddr) {
		asm volatile("invlpg %0" : : "m"(*(uint8_t*)vaddr) : "memory");
	}

	void parse_mboot_memory_map(struct multiboot_info* header, struct multiboot_mmap_entry* mmap_entry) {
		static
		size_t mmap_offset = 0;
		usable_bytes_ram = 0;
		while(mmap_offset < header->mmap_length) {
			if(mmap_entry->addr_high || mmap_entry->len_high) {
				//If the entry is in extended memory, ignore it
				printf("[kinit] Ignoring memory region above 4GiB (0x%x%x)\n",
						mmap_entry->addr_high, mmap_entry->addr_low);
			} else {
				//Otherwise, round up the address of the entry to a page boundary and round the size down to a page boundary
				MemoryRegion& region = multiboot_memory_regions[num_multiboot_memory_regions];
				uint32_t addr_pagealigned = ((mmap_entry->addr_low + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
				uint32_t size_pagealigned = ((mmap_entry->len_low - (addr_pagealigned - mmap_entry->addr_low)) / PAGE_SIZE) * PAGE_SIZE;
				if(size_pagealigned) {
					//If the page-aligned size is more than zero (eg mmap_entry->len >= PAGE_SIZE), interpret it
					region = MemoryRegion(addr_pagealigned, size_pagealigned);
					region.heap_allocated = false;
					region.reserved = mmap_entry->type == MULTIBOOT_MEMORY_RESERVED;
					region.used = mmap_entry->type != MULTIBOOT_MEMORY_AVAILABLE;
					total_bytes_ram += region.size;
					if(!region.used) usable_bytes_ram += region.size;
					if(region.reserved) reserved_bytes_ram += region.size;
					if(mmap_entry->type == MULTIBOOT_MEMORY_BADRAM) bad_bytes_ram += region.size;
					num_multiboot_memory_regions++;
					printf("[kinit] Adding memory region from 0x%x -> 0x%x (%s, %s)\n", region.start, region.start + (region.size - 1), region.used ? "Used" : "Unused", region.reserved ? "Reserved" : "Unreserved");
				} else {
					//Otherwise, ignore it
					printf("[kinit] Ignoring too-small memory region at 0x%x\n", mmap_entry->addr_low);
				}
			}
			mmap_offset += mmap_entry->size + sizeof(mmap_entry->size);
			mmap_entry = (struct multiboot_mmap_entry*) ((size_t)mmap_entry + mmap_entry->size + sizeof(mmap_entry->size));
		}

		//Create the linked list of memory regions
		for(auto i = 0; i < num_multiboot_memory_regions; i++) {
			MemoryRegion& region = multiboot_memory_regions[i];
			region.prev = i ? &multiboot_memory_regions[i - 1] : nullptr;
			region.next = i < num_multiboot_memory_regions - 1 ? &multiboot_memory_regions[i + 1] : nullptr;
		}
	}

}

using namespace Memory;

int liballoc_lock() {
	cli();
	return 0;
}

int liballoc_unlock() {
	sti();
	return 0;
}

bool _ALLOC_ENABLED = true;
void *liballoc_alloc(int pages) {
	PageDirectory::used_kheap_pmem += pages * PAGE_SIZE;
	_ALLOC_ENABLED = false;
	auto* ret = PageDirectory::k_alloc_region_for_heap(pages * PAGE_SIZE);
	_ALLOC_ENABLED = true;
	return ret;
}

void liballoc_afteralloc(void* ptr_alloced) {
	PageDirectory::k_after_alloc();
}

int liballoc_free(void *ptr, int pages) {
	PageDirectory::used_kheap_pmem -= pages * PAGE_SIZE;
	PageDirectory::k_free_region(ptr);
	return 0;
}
