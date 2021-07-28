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

#ifndef PAGING_H
#define PAGING_H

#include <kernel/kstd/types.h>

#define PAGING_4KiB 0
#define PAGING_4MiB 1
#define PAGE_SIZE 4096
#define PAGE_SIZE_FLAG PAGING_4KiB
#define HIGHER_HALF 0xC0000000
#define KERNEL_TEXT ((size_t)&_KERNEL_TEXT)
#define KERNEL_TEXT_END ((size_t)&_KERNEL_TEXT_END)
#define KERNEL_DATA ((size_t)&_KERNEL_DATA)
#define KERNEL_DATA_END ((size_t)&_KERNEL_DATA_END)
#define KERNEL_END ((size_t)&_KERNEL_END)
#define PAGETABLES_START ((size_t)&_PAGETABLES_START)
#define PAGETABLES_END ((size_t)&_PAGETABLES_END)
#define KERNEL_TEXT_SIZE (KERNEL_TEXT_END - KERNEL_TEXT)
#define KERNEL_DATA_SIZE (KERNEL_DATA_END - KERNEL_DATA)
#define KERNEL_END_VIRTADDR (HIGHER_HALF + KERNEL_SIZE_PAGES * PAGE_SIZE)

/**
 * The basic premise of how the memory allocation in duckOS is as follows:
 *
 * The kernel is always mapped to the topmost 1GiB of the address space. All the relevant page directory entries for
 * the kernel are stored in static variables in the PageDirectory class, and are copied over to each new page directory.
 *
 * There is a bitmap that dictates which 4KiB pages in physical memory are used (in paging.cpp), and each page directory
 * has a bitmap dictating which virtual 4KiB pages are used. Additionally, there is a static virtual memory bitmap for
 * the kernel.
 *
 * When a process needs to allocate a new page, the OS first looks to see if the page table which would handle that
 * virtual address even exists yet. If it doesn't, a 4KiB page to store it is allocated and mapped to
 * PAGETABLES_VIRTADDR in the kernel space (but since this mapping is per-process, it is not stored with the
 * static kernel entries). If all of the entries in a page table go unused, the page table is deallocated to free up
 * physical memory.
 *
 * This system means there is very little overhead when multitasking; when the context is switched, we simply load the
 * page directory for the current process and we go on our way. If for any reason the kernel's page directory entries
 * are modified, every process's page directory is updated to reflect the change (although this shouldn't happen very
 * often)
 */

class PageTable;
class PageDirectory;
class MemoryMap;

extern "C" long _KERNEL_TEXT;
extern "C" long _KERNEL_TEXT_END;
extern "C" long _KERNEL_DATA;
extern "C" long _KERNEL_DATA_END;
extern "C" long _PAGETABLES_START;
extern "C" long _PAGETABLES_END;

struct multiboot_info;
struct multiboot_mmap_entry;

namespace Memory {
	extern PageDirectory kernel_page_directory;
	extern "C" void load_page_dir(size_t* dir);

	/**
	 * Sets up paging.
	 */
	void setup_paging();

	/**
	 * Called when the CPU encounters a page fault in the kernel.
	 * @param r the Registers struct from the isr.
	 */
	void page_fault_handler(struct Registers *r);

	/**
	 * Get the physical memory bitmap.
	 * @return The physical memory bitmap.
	 */
	MemoryMap& pmem_map();

	/**
	 * Used when setting up paging initially in order to map an entire page table starting at a virtual address.
	 * @param page_table The page table to set up
	 * @param virtual_address The virtual address to map the pagetable to.
	 * @param read_write The read_write flag on the page tables.
	 */
	void early_pagetable_setup(PageTable* page_table, size_t virtual_address, bool read_write);

	/**
	 * Get the amount of used physical memory in bytes. Doesn't include reserved memory.
	 * @return The amount of allocated physical memory in bytes (granularity of PAGE_SIZE)
	 */
	size_t get_used_mem();

	/**
	 * Get the amount of reserved physical memory in bytes.
	 * @return The amount of reserved physical memory in bytes (granularity of PAGE_SIZE)
	 */
	size_t get_reserved_mem();

	/**
	 * Get the total amount of usable physical memory in bytes.
	 * @return The amount of usable physical memory in bytes.
	 */
	 size_t get_usable_mem();

	/**
	 * Get the amount of vmem used by the kernel in bytes.
	 * @return The amount of virtual memory the kernel is using (granularity of PAGE_SIZE)
	 */
	size_t get_kernel_vmem();

	/**
	 * Get the amount of pmem used by the kernel in bytes. (This includes the kernel's code + heap)
	 * @return The amount of physical memory the kernel is using (granularity of PAGE_SIZE)
	 */
	size_t get_kernel_pmem();

	/**
	 * Get the amount of pmem used by the kernel heap in bytes.
	 * @return The amount of memory the kernel is using (granularity of PAGE_SIZE)
	 */
	size_t get_kheap_pmem();

	/**
	 * Invalidates the page in the TLB that contains vaddr.
	 * @param vaddr A pointer that is the vaddr being invalidated.
	 */
	 void invlpg(void* vaddr);

	 /**
	  * Parses the multiboot memory map.
	  */
	 void parse_mboot_memory_map(multiboot_info* header, multiboot_mmap_entry* first_entry);
};

int liballoc_lock();
int liballoc_unlock();
void* liballoc_alloc(int);
void liballoc_afteralloc(void* ptr_alloced);
int liballoc_free(void*,int);

#endif