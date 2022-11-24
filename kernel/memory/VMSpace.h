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
	VMSpace(VirtualAddress start, size_t size, PageDirectory& page_directory);
	~VMSpace();

	const static VMProt default_prot;

	/**
	 * Allocates a new region for the given object.
	 * @param object The object to allocate a region for.
	 * @return The newly created region.
	 */
	ResultRet<Ptr<VMRegion>> map_object(Ptr<VMObject> object, VMProt prot = VMSpace::default_prot);

	/**
	 * Maps an object into a specific area of the address space.
	 * @param object The object to map into the space.
	 * @param addr The address to map the object into.
	 * @return The newly mapped region.
	 */
	ResultRet<Ptr<VMRegion>> map_object(Ptr<VMObject> object, VirtualAddress address, VMProt prot = VMSpace::default_prot);

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
	ResultRet<VMRegion*> get_region(VirtualAddress address);

	/**
	 * Reserves a region so it cannot be allocated. Should be page-aligned.
	 * @param start The start of the region.
	 * @param size The size of the region.
	 * @return Whether the region was succesfully reserved.
	 */
	Result reserve_region(VirtualAddress start, size_t size);

	VirtualAddress start() const { return m_start; }
	size_t size() const { return m_size; }
	VirtualAddress end() const { return m_start + m_size; }
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
		bool contains(VirtualAddress address) const { return start <= address && end() > address; }
	};

	VirtualAddress m_start;
	size_t m_size;
	kstd::vector<VMRegion*> m_regions;
	VMSpaceRegion* m_region_map;
	size_t m_used = 0;
	SpinLock m_lock;
	PageDirectory& m_page_directory;
};
