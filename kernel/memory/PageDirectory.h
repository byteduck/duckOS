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

#include <kernel/kstd/unix_types.h>
#include <kernel/tasking/SpinLock.h>
#include <kernel/Result.hpp>
#include "Memory.h"
#include "VMRegion.h"

class PageTable;

class PageDirectory {
public:
	enum class DirectoryType {
		USER,
		KERNEL
	};

	typedef union Entry {
		class __attribute((packed)) Data {
		public:
			bool present : 1;
			bool read_write : 1;
			bool user : 1;
			bool write_through : 1;
			bool cache_disable : 1;
			bool accessed : 1;
			bool zero : 1;
			uint8_t size : 1;
			bool ignored : 1;
			uint8_t unused : 3;
			size_t page_table_addr : 20;

			void set_address(size_t address);
			size_t get_address();
		} data;
		uint32_t value;
	} Entry;

	/**
	 * Initialize the kernel page directory entries & related variables.
	 */
	static void init_paging();

	explicit PageDirectory(DirectoryType type = DirectoryType::USER);
	~PageDirectory();

	/**
	 * Gets a pointer to the entries in the PageDirectory.
	 * @return
	 */
	Entry* entries();

	/**
	 * Get the physical address of the table of entries.
	 * @return The physical address of the table of entries.
	 */
	size_t entries_physaddr();

	/** Maps a region into the page directory. **/
	void map(VMRegion& region);

	/** Unmaps a region from the page directory. **/
	void unmap(VMRegion& region);

	/**
	 * Gets the physical address for virtaddr.
	 * @param virtaddr The virtual address.
	 * @return The physical address for virtaddr.
	 */
	size_t get_physaddr(VirtualAddress virtaddr);

	/**
	 * Calls get_physaddr(size_t virtaddr).
	 */
	size_t get_physaddr(void* virtaddr);

	/**
	 * Allocates space for a new page table at tables_index in the page directory.
	 * @param tables_index The index in the page directory that this newly allocated page table is for.
	 * @return A pointer to the newly allocated page table.
	 */
	PageTable* alloc_page_table(size_t tables_index);

	/**
	 * Deallocates the space used for a page table at tables_index in the page directory.
	 * @param tables_index The index in the page directory of the page table being dealloc'd.
	 */
	void dealloc_page_table(size_t tables_index);

	/**
	 * Checks if a given virtual address is mapped to anything.
	 * @param vaddr The virtual address to check.
	 * @param permission Whether to check for write permission.
	 * @return Whether or not the given virtual address is mapped.
	 */
	bool is_mapped(VirtualAddress vaddr, bool write);

	/**
	 * Gets whether or not this PageDirectory is currently mapped.
	 * @return Whether or not the PageDirectory is currently mapped.
	 */
	bool is_mapped();

private:
	friend class MemoryManager;
	/**
	 * Maps a virtual page to a physical page.
	 * @param vpage The index of the virtual page to map.
	 * @param ppage The index of the physical page to map it to.
	 * @param prot The protection to map the page with.
	 * @return Whether the page was successfully mapped.
	 */
	Result map_page(PageIndex vpage, PageIndex ppage, VMProt prot);

	/**
	 * Unmaps a virtual page.
	 * @param vpage The index of the virtual page to unmap.
	 * @return Whether the page was successfully unmapped.
	 */
	Result unmap_page(PageIndex vpage);

	// The entries for the kernel.
	static Entry s_kernel_entries[1024];
	// The page tables for the kernel.
	static PageTable s_kernel_page_tables[256];

	// The type of the page directory.
	const DirectoryType m_type;
	// The VMRegion for this page directory's entries.
	kstd::Arc<VMRegion> m_entries_region;
	// The page directory entries for this page directory.
	Entry* m_entries = nullptr;
	// An array of pointers to the page tables that the directory points to.
	PageTable* m_page_tables[768] = {nullptr};
	// An array of u16s that stores the number of pages mapped in each page table, used to deallocate a page table once no longer needed
	volatile int m_page_tables_num_mapped[1024] = {0};
	// A lock used to prevent race conditions.
	SpinLock m_lock;
};

