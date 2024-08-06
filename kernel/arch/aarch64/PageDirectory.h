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
#include <kernel/tasking/Mutex.h>
#include <kernel/Result.hpp>
#include <kernel/memory/Memory.h>
#include <kernel/memory/VMRegion.h>
#include <kernel/arch/aarch64/MMU.h>

class PageTable;

class PageDirectory {
public:
	enum class DirectoryType {
		USER,
		KERNEL
	};

	explicit PageDirectory(DirectoryType type = DirectoryType::USER);
	~PageDirectory();

	/**
	 * Initialize the kernel page directory entries & related variables.
	 */
	static void init_paging();

	/**
	 * Maps a portion of a region into the page directory.
	 * @param region The region to map.
	 * @param range The range within the region to map relative to the start of the region. Use VirtualRange::null to map the whole region.
	 */
	void map(VMRegion& region, VirtualRange range = VirtualRange::null);

	/**
	 * Unmaps a portion of a region from the page directory.
	 * @param region The region to map.
	 * @param range The range within the region to unmap relative to the start of the region. Use VirtualRange::null to unmap the whole region.
	 */
	void unmap(VMRegion& region, VirtualRange range = VirtualRange::null);

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
	 * @param is_device Whether the memory is device memory.
	 * @return Whether the page was successfully mapped.
	 */
	Result map_page(PageIndex vpage, PageIndex ppage, VMProt prot, bool is_device = false);

	/**
	 * Unmaps a virtual page.
	 * @param vpage The index of the virtual page to unmap.
	 * @return Whether the page was successfully unmapped.
	 */
	Result unmap_page(PageIndex vpage);

	/**
	 * Maps the kernel when booting, without initializing VMRegions and tracking physical pages.
	 */
	static void setup_kernel_map();

	/**
	 * Allocates a page table.
	 */
	static kstd::Arc<VMRegion> alloc_table();

	/* Yes, these could all be the same struct.
	 * However, this way, naming in code will be much clearer. */
	struct LowerPages {
		Aarch64::MMU::PageDescriptor* entries;
		kstd::Arc<VMRegion> region;
	};

	template<typename Child>
	struct Table {
		Aarch64::MMU::TableDescriptor* entries = nullptr;
		kstd::Arc<Child> children[512] = {kstd::Arc<Child>(nullptr)};
		kstd::Arc<VMRegion> region;

		kstd::Arc<Child> get_child(size_t index) {
			ASSERT(index >= 0 && index < 512);
			auto& child = children[index];
			if (__builtin_expect(!child, false)) {
				child = kstd::Arc<Child>(new Child());

				// Alloc space for the table. Try early bump alloc first if we still have space.
				child->entries = (typeof(child->entries)) Aarch64::MMU::alloc_early_table();
				if (!child->entries) {
					child->region = alloc_table();
					ASSERT(child->region);
					child->entries = (typeof(child->entries)) child->region->start();
					entries[index].address = child->region->object()->physical_page_index(0);
				} else {
					entries[index].address = Aarch64::MMU::descriptor_addr(((size_t) child->entries) - HIGHER_HALF);
				}

				entries[index].valid = true;
				entries[index].type = Aarch64::MMU::TableDescriptor::Table;
				entries[index].heirarchical_perms = 0;
				entries[index].security = Aarch64::MMU::TableDescriptor::Secure;
			}
			return child;
		}

		kstd::Arc<Child> get_child_if_exists(size_t index) {
			ASSERT(index >= 0 && index < 512);
			return children[index];
		}
	};

	using MiddleTable = Table<LowerPages>;
	using UpperTable = Table<MiddleTable>;
	using GlobalTable = Table<UpperTable>;

	// The page global directory.
	GlobalTable m_global_table;

	// The start page of the global directory.
	PageIndex start_page;

	// The type of the page directory.
	const DirectoryType m_type;

	// A lock used to prevent race conditions.
	Mutex m_lock {"PageDirectory"};
};