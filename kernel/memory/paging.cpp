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

#include <kernel/kstddef.h>
#include <kernel/kstdio.h>
#include <kernel/memory/paging.h>
#include <kernel/interrupt/isr.h>
#include <kernel/memory/MemoryBitmap.hpp>
#include "PageDirectory.h"
#include "PageTable.h"

namespace Paging {
	PageDirectory kernel_page_directory;
	PageDirectory::Entry kernel_page_directory_entries[1024] __attribute__((aligned(4096)));
	PageTable::Entry kernel_early_page_table_entries[1024] __attribute__((aligned(4096)));
	MemoryBitmap<0x100000> _pmem_bitmap;
	size_t usable_bytes_ram = 0;

	//TODO: Assumes computer has at least 4GiB of memory. Should detect memory in future.
	void setup_paging() {
		//Assert that the kernel doesn't exceed 7MiB
		ASSERT(KERNEL_END - KERNEL_START <= 0x700000);

		kernel_page_directory.set_entries(kernel_page_directory_entries);
		PageDirectory::init_kmem();

		//Mark the kernel's pages as used
		size_t k_page_start = (KERNEL_START - HIGHER_HALF) / PAGE_SIZE;
		size_t k_page_end = k_page_start + KERNEL_SIZE_PAGES;
		for (auto i = k_page_start; i < k_page_end; i++) {
			pmem_bitmap().set_page_used(i);
		}

		//Setup kernel page directory to map the kernel to HIGHER_HALF
		PageTable kernel_early_page_table(0, nullptr, false);
		kernel_early_page_table.entries() = kernel_early_page_table_entries;
		early_pagetable_setup(&kernel_early_page_table, HIGHER_HALF, true);
		for (auto i = 0; i < 1024; i++) {
			kernel_early_page_table[i].data.present = true;
			kernel_early_page_table[i].data.read_write = true;
			kernel_early_page_table[i].data.set_address(PAGE_SIZE * i);
		}

		//Enable paging
		asm volatile(
				"movl %%eax, %%cr3\n" //Put the page directory pointer in cr3
				"movl %%cr0, %%eax\n"
				"orl $0x80000000, %%eax\n" //Set the proper flags in cr0
				"movl %%eax, %%cr0\n"
				: : "a"((size_t) kernel_page_directory.entries() - HIGHER_HALF)
		);

		//Map kernel pages into page_tables
		PageDirectory::k_map_pages(KERNEL_START - HIGHER_HALF, KERNEL_START, true, KERNEL_SIZE_PAGES);

		//Map the page with the video memory in it (in kstdio.c/h)
		set_graphical_mode(false);

		//Now, write everything to the directory
		kernel_page_directory.update_kernel_entries();
	}


	MemoryBitmap<0x100000>& pmem_bitmap() {
		return _pmem_bitmap;
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
		asm("mov %%cr2, %0" : "=r" (err_pos));
		printf("Virtual address: 0x%X\n\n", err_pos);
		print_regs(r);
		while (true);
	}


	size_t get_used_mem() {
		return (pmem_bitmap().used_pages() * PAGE_SIZE) / 1024;
	}

	size_t get_total_mem() {
		return usable_bytes_ram / 1024;
	}

	size_t get_used_kmem() {
		return (PageDirectory::kernel_vmem_bitmap.used_pages() * PAGE_SIZE) / 1024;
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

	//TODO: Actually use the map
	void parse_mboot_memory_map(struct multiboot_info* header, struct multiboot_mmap_entry* mmap_entry) {
		size_t i = 0;
		usable_bytes_ram = 0;
		while(i < header->mmap_length) {
			i += mmap_entry->size + sizeof(mmap_entry->size);
			usable_bytes_ram += mmap_entry->len;
			mmap_entry = (struct multiboot_mmap_entry*) ((size_t)mmap_entry + mmap_entry->size + sizeof(mmap_entry->size));
		}
	}

}

using namespace Paging;

int liballoc_lock() {
	cli();
	return 0;
}

int liballoc_unlock() {
	sti();
	return 0;
}

void *liballoc_alloc(int pages) {
	return PageDirectory::k_alloc_pages(pages * PAGE_SIZE);
}

int liballoc_free(void *ptr, int pages) {
	PageDirectory::k_free_pages(ptr, pages * PAGE_SIZE);
	return 0;
}
