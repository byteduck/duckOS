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

#pragma once

#include <kernel/kstd/types.h>
#include <kernel/kstd/Arc.h>
#include "PageTable.h"
#include "PageDirectory.h"
#include "PhysicalPage.h"
#include "PhysicalRegion.h"
#include "BuddyZone.h"
#include "VMSpace.h"
#include <kernel/tasking/SpinLock.h>
#include "Memory.h"

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

extern "C" long _KERNEL_TEXT;
extern "C" long _KERNEL_TEXT_END;
extern "C" long _KERNEL_DATA;
extern "C" long _KERNEL_DATA_END;
extern "C" long _PAGETABLES_START;
extern "C" long _PAGETABLES_END;

struct multiboot_info;
struct multiboot_mmap_entry;

#define MM MemoryManager::inst()

class MemoryManager {
public:
	PageDirectory kernel_page_directory { PageDirectory::DirectoryType::KERNEL };

	PageDirectory::Entry kernel_page_directory_entries[1024] __attribute__((aligned(4096)));
	PageTable::Entry kernel_early_page_table_entries1[1024] __attribute__((aligned(4096)));
	PageTable::Entry kernel_early_page_table_entries2[1024] __attribute__((aligned(4096)));

	SpinLock liballoc_spinlock;

	MemoryManager();

	static MemoryManager& inst();

	/**
	 * Sets up paging.
	 */
	void setup_paging();

	/**
	 * Loads a page directory.
	 */
	void load_page_directory(const kstd::Arc<PageDirectory>& page_directory);
	void load_page_directory(PageDirectory* page_directory);
	void load_page_directory(PageDirectory& page_directory);

	/**
	 * Called when the CPU encounters a page fault in the kernel.
	 * @param r the Registers struct from the isr.
	 */
	void page_fault_handler(struct Registers *r);

	/** Gets a reference to the given physical page (indexed by page number, NOT address.) **/
	PhysicalPage& get_physical_page(size_t page_number) const {
		return m_physical_pages[page_number];
	}

	/** Allocates a physical page for use. The resulting page will have a refcount of 1. **/
	ResultRet<PageIndex> alloc_physical_page() const;

	/** Allocates non-contiguous physical pages for use. The resulting pages will have a refcount of 1. **/
	ResultRet<kstd::vector<PageIndex>> alloc_physical_pages(size_t num_pages) const;

	/** Allocates contiguous physical pages for use. The resulting pages will have a refcount of 1. **/
	ResultRet<kstd::vector<PageIndex>> alloc_contiguous_physical_pages(size_t num_pages) const;

	/**
	 * Allocates a new non-contiguous anonymous region in kernel space.
	 * @param size The minimum size, in bytes, of the new region.
	 */
	kstd::Arc<VMRegion> alloc_kernel_region(size_t size);

	/**
	 * Allocates a new contiguous anonymous region in kernel space.
	 * @param size The minimum size, in bytes, of the new region.
	 */
	kstd::Arc<VMRegion> alloc_dma_region(size_t size);

	/**
	 * Allocates a new virtual region in kernel space that is mapped to an existing range of physical pages.
	 * @param start The start physical address to map to. Will be rounded down to a page boundary.
	 * @param size The size (in bytes) to map to. Will be rounded up to a page boundary.
	 * @return The newly mapped region.
	 */
	kstd::Arc<VMRegion> alloc_mapped_region(PhysicalAddress start, size_t size);

	/**
	 * Maps a VMObject into kernel space.
	 * @param object The object to map.
	 * @return The region the object was mapped to.
	 */
	kstd::Arc<VMRegion> map_object(kstd::Arc<VMObject> object);

	kstd::Arc<VMSpace> kernel_space() { return m_kernel_space; }

	/**
	 * Frees a physical page for future use. The page should have a refcount of 0.
	 * The preferred method of freeing a physical page is by calling deref() on it, which will free it once it has zero
	 * references.
	 */
	void free_physical_page(PageIndex page) const;

	/**
	 * Invalidates the page in the TLB that contains vaddr.
	 * @param vaddr A pointer that is the vaddr being invalidated.
	 */
	 void invlpg(void* vaddr);

	/**
	 * Parses the multiboot memory map.
	 */
	void parse_mboot_memory_map(multiboot_info* header, multiboot_mmap_entry* first_entry);

	/**
	 * Allocates a number of new pages for the heap. `finalize_heap_pages` MUST be called afterwards to release the lock
	 * on the VMSpace and finalize the allocation.
	 * @param num_pages The number of pages to allocate for the heap.
	 * @return The virtual address of the first page allocated.
	 */
	ResultRet<VirtualAddress> alloc_heap_pages(size_t num_pages);

	/**
	 * See `alloc_heap_pages`.
	 */
	void finalize_heap_pages();

	// Various usage statistics
	size_t usable_mem() const;
	size_t used_pmem() const;
	size_t reserved_pmem() const;
	size_t kernel_vmem() const;
	size_t kernel_pmem() const;
	size_t kernel_heap() const;

private:
	friend class PhysicalRegion;

	static MemoryManager* _inst;

	// Heap stuff
	kstd::vector<PageIndex> m_heap_pages = kstd::vector<PageIndex>(1024);
	size_t m_num_heap_pages;
	VirtualAddress m_last_heap_loc;
	SpinLock m_heap_lock;

	PhysicalPage* m_physical_pages;
	kstd::vector<PhysicalRegion*> m_physical_regions;
	kstd::Arc<VMSpace> m_kernel_space;
};

void liballoc_lock();
void liballoc_unlock();
void* liballoc_alloc(int);
void liballoc_afteralloc(void* ptr_alloced);
void liballoc_free(void*,int);