/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once
#include "Memory.h"
#include "../kstd/shared_ptr.hpp"
#include "VMRegion.h"
#include "../Result.hpp"
#include "../tasking/SpinLock.h"
#include "PageDirectory.h"

/**
 * This class represents a virtual memory address space and all of the regions it contains. It's used to allocate and
 * map new regions in virtual memory.
 */
class VMSpace {
public:
	VMSpace(VirtualAddress start, size_t size);
	~VMSpace();

	/**
	 * Allocates a new region for the given object.
	 * @param object The object to allocate a region for.
	 * @return The newly created region.
	 */
	ResultRet<kstd::shared_ptr<VMRegion>> map_object(kstd::shared_ptr<VMObject> object);

	/**
	 * Maps an object into a specific area of the address space.
	 * @param object The object to map into the space.
	 * @param addr The address to map the object into.
	 * @return The newly mapped region.
	 */
	ResultRet<kstd::shared_ptr<VMRegion>> map_object(kstd::shared_ptr<VMObject> object, VirtualAddress address);

	/**
	 * Unmaps the given region from this address space.
	 * @param region The region to unmap.
	 * @return Whether the region was succesfully unmapped.
	 */
	Result unmap_region(kstd::shared_ptr<VMRegion> region);

	VirtualAddress start() const { return m_start; }
	size_t size() const { return m_size; }
	size_t used() const { return m_used; }

private:

	ResultRet<VirtualAddress> alloc_space(size_t size);
	ResultRet<VirtualAddress> alloc_space_at(size_t size, VirtualAddress address);
	Result free_space(size_t size, VirtualAddress address);

	struct VMSpaceRegion {
		VirtualAddress start;
		size_t size;
		bool used;
		VMSpaceRegion* next;
		VMSpaceRegion* prev;

		size_t end() const { return start + size; }
		bool contains(VirtualAddress address, size_t range_size) const { return start <= address && end() >= start + range_size; }
	};

	VirtualAddress m_start;
	size_t m_size;
	kstd::vector<kstd::shared_ptr<VMRegion>> m_regions;
	VMSpaceRegion* m_region_map;
	size_t m_used = 0;
	SpinLock m_lock;
	PageDirectory m_page_directory;
};
