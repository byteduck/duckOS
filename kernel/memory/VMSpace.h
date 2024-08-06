/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once
#include "Memory.h"
#include "../kstd/Arc.h"
#include "VMRegion.h"
#include "../Result.hpp"
#include "../tasking/Mutex.h"
#include <kernel/memory/PageDirectory.h>
#include <kernel/kstd/Iteration.h>

/**
 * This class represents a virtual memory address space and all of the regions it contains. It's used to allocate and
 * map new regions in virtual memory.
 */
class VMSpace: public kstd::ArcSelf<VMSpace> {
public:
	VMSpace(VirtualAddress start, size_t size, PageDirectory& page_directory);
	~VMSpace();

	const static VMProt default_prot;

	/**
	 * Forks this VMSpace into a new VMSpace, cloning or CoW-ing mapped objects as necessary.
	 * @param page_directory The new page directory to use.
	 * @param regions_vec A reference to a vector to store the resulting VMRegions in.
	 * @return The newly forked VMSpace.
	 */
	kstd::Arc<VMSpace> fork(PageDirectory& page_directory, kstd::vector<kstd::Arc<VMRegion>>& regions_vec);

	/**
	 * Allocates a new region for the given object.
	 * @param object The object to allocate a region for.
	 * @param prot The protection to use.
	 * @param range The range within the space to map to. Use a start of zero to map wherever it fits, and a size of zero to map the whole object. Both must be page-aligned.
	 * @param object_start The offset within the object to begin the mapping. Must be page-aligned.
	 * @return The newly created region.
	 */
	ResultRet<kstd::Arc<VMRegion>> map_object(kstd::Arc<VMObject> object, VMProt prot, VirtualRange range = VirtualRange::null, VirtualAddress object_start = 0);

	/**
	 * Allocates a new region for the given object with sentinel pages on either side.
	 * @param object The object to allocate a region for.
	 * @param prot The protection to use.
	 * @return The newly created region.
	 */
	ResultRet<kstd::Arc<VMRegion>> map_object_with_sentinel(kstd::Arc<VMObject> object, VMProt prot);

	/**
	 * Allocates a new region for the given object near the end of the memory space.
	 * @param object The object to allocate a region for.
	 * @return The newly created region.
	 */
	ResultRet<kstd::Arc<VMRegion>> map_stack(kstd::Arc<VMObject> object, VMProt prot = VMSpace::default_prot);

	/**
	 * Unmaps the given region from this address space.
	 * @param region The region to unmap.
	 * @return Whether the region was succesfully unmapped.
	 */
	Result unmap_region(VMRegion& region);

	/**
	 * Unmaps the region at the given address.
	 * @param region The address of the region to unmap.
	 * @return Whether the region was succesfully unmapped.
	 */
	Result unmap_region(VirtualAddress address);

	/**
	 * Gets the region at the given address
	 * @param address The address of the region to find.
	 * @return The region, if found.
	 */
	ResultRet<kstd::Arc<VMRegion>> get_region_at(VirtualAddress address);

	/**
	 * Gets the region containing the given address
	 * @param address The address contained within the region to find.
	 * @return The region, if found.
	 */
	ResultRet<kstd::Arc<VMRegion>> get_region_containing(VirtualAddress address);

	/**
	 * Reserves a region so it cannot be allocated. Should be page-aligned.
	 * @param start The start of the region.
	 * @param size The size of the region.
	 * @return Whether the region was succesfully reserved.
	 */
	Result reserve_region(VirtualAddress start, size_t size);

	/**
	 * Tries gracefully handling a pagefault.
	 * @param fault The page fault.
	 * @return A result indicating whether the pagefault could be gracefully handled.
	 */
	Result try_pagefault(PageFault fault);

	/**
	 * Finds a region in the space that has at least `size` bytes free.
	 * SHOULD ONLY BE USED BY `MemoryManager` FOR HEAP ALLOCATION.
	 *
	 * @param size The size needed.
	 * @return An address to the start of an unused region with adequate space, or an error if not found.
	 */
	ResultRet<VirtualAddress> find_free_space(size_t size);

	/**
	 * Calculates the total non-shared, anonymous memory in the space.
	 */
	size_t calculate_regular_anonymous_total();

	/**
	 * Iterates over all VMRegions in the space.
	 */
	void iterate_regions(kstd::IterationFunc<VMRegion*> callback);

	VirtualAddress start() const { return m_start; }
	size_t size() const { return m_size; }
	VirtualAddress end() const { return m_start + m_size; }
	size_t used() const { return m_used; }
	Mutex& lock() { return m_lock; }

private:
	struct VMSpaceRegion {
		VirtualAddress start;
		size_t size;
		bool used;
		VMSpaceRegion* next;
		VMSpaceRegion* prev;
		VMRegion* vmRegion;

		size_t end() const { return start + size; }
		bool contains(VirtualAddress address) const { return start <= address && end() > address; }
	};

	ResultRet<VMSpaceRegion*> alloc_space(size_t size);
	ResultRet<VMSpaceRegion*> alloc_space_at(size_t size, VirtualAddress address);
	Result free_region(VMSpaceRegion* region);

	VirtualAddress m_start;
	size_t m_size;
	VMSpaceRegion* m_region_map;
	size_t m_used = 0;
	Mutex m_lock {"VMSpace"};
	PageDirectory& m_page_directory;
};
