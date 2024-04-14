/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "VMSpace.h"
#include "MemoryManager.h"
#include "AnonymousVMObject.h"
#include "../kstd/cstring.h"
#include "InodeVMObject.h"
#include "../kstd/KLog.h"

const VMProt VMSpace::default_prot = {
	.read = true,
	.write = true,
	.execute = true
};

VMSpace::VMSpace(VirtualAddress start, size_t size, PageDirectory& page_directory):
	m_start(start),
	m_size(size),
	m_region_map(new VMSpaceRegion {.start = start, .size = size, .used = false, .next = nullptr, .prev = nullptr}),
	m_page_directory(page_directory)
{}

VMSpace::~VMSpace() {
	auto cur_region = m_region_map;
	while(cur_region) {
		auto next = cur_region->next;
		delete cur_region;
		cur_region = next;
	}
}

kstd::Arc<VMSpace> VMSpace::fork(PageDirectory& page_directory, kstd::vector<kstd::Arc<VMRegion>>& regions_vec) {
	LOCK(m_lock);
	auto new_space = kstd::Arc<VMSpace>(new VMSpace(m_start, m_size, page_directory));
	new_space->m_used = m_used;
	delete new_space->m_region_map;

	// Clone regions
	auto cur_region = m_region_map;
	VMSpaceRegion* prev_new_region = nullptr;
	while(cur_region) {
		// Clone the VMSpaceRegion
		auto new_region = new VMSpaceRegion(*cur_region);
		if(cur_region == m_region_map)
			new_space->m_region_map = new_region;
		new_region->prev = prev_new_region;
		if(prev_new_region)
			prev_new_region->next = new_region;
		prev_new_region = new_region;

		// Clone the vmRegion
		if(cur_region->vmRegion) {
			auto region = cur_region->vmRegion;
			kstd::Arc<VMObject> new_object = region->object();
			// Mark as CoW / share if necessary
			switch(region->object()->fork_action()) {
				case VMObject::ForkAction::BecomeCoW: {
					auto new_object_res = region->object()->clone();
					m_page_directory.map(*region);
					if(new_object_res.is_error()) {
						KLog::err("VMSpace", "Could not clone a VMObject: {}!", new_object_res.code());
						break;
					}
					new_object = new_object_res.value();
					[[fallthrough]];
				}
				case VMObject::ForkAction::Share: {
					auto new_vmRegion = kstd::Arc<VMRegion>::make(
							new_object,
							new_space,
							region->range(), region->object_start(),
							region->prot());
					page_directory.map(*new_vmRegion);
					new_region->vmRegion = new_vmRegion.get();
					regions_vec.push_back(new_vmRegion);
					break;
				}
				case VMObject::ForkAction::Ignore:
					break;
			}
		}

		cur_region = cur_region->next;
	}

	return new_space;
}

ResultRet<kstd::Arc<VMRegion>> VMSpace::map_object(kstd::Arc<VMObject> object, VMProt prot, VirtualRange range, VirtualAddress object_start) {
	// Use the size of the range if defined, or the remainder of the object size if not.
	if(!range.size)
		range.size = object->size() - object_start;

	// Validate alignments and size
	if(range.start % PAGE_SIZE != 0 || range.size % PAGE_SIZE != 0 || object_start % PAGE_SIZE != 0 || object_start + range.size > object->size())
		return Result(EINVAL);

	// Allocate the space region appropriately
	VMSpaceRegion* region;
	if(range.start)
		region = TRY(alloc_space_at(range.size, range.start));
	else
		region = TRY(alloc_space(range.size));

	// Create and map the region
	auto vmRegion = kstd::make_shared<VMRegion>(
			object,
			self(),
			VirtualRange {region->start, range.size},
			object_start,
			prot);
	region->vmRegion = vmRegion.get();
	m_page_directory.map(*vmRegion);
	return vmRegion;
}

ResultRet<kstd::Arc<VMRegion>> VMSpace::map_object_with_sentinel(kstd::Arc<VMObject> object, VMProt prot) {
	// Create and map the region
	VMSpaceRegion* region = TRY(alloc_space(object->size() + PAGE_SIZE * 2));
	auto vmRegion = kstd::make_shared<VMRegion>(
			object,
			self(),
			VirtualRange {region->start + PAGE_SIZE, object->size()},
			0,
			prot);
	region->vmRegion = vmRegion.get();
	m_page_directory.map(*vmRegion);
	return vmRegion;
}

ResultRet<kstd::Arc<VMRegion>> VMSpace::map_stack(kstd::Arc<VMObject> object, VMProt prot) {
	LOCK(m_lock);

	// Find the endmost region with space in it
	auto cur_region = m_region_map;
	while(cur_region->next)
		cur_region = cur_region->next;
	while((cur_region->used || cur_region->size < object->size()) && cur_region)
		cur_region = cur_region->prev;
	if(!cur_region)
		return Result(ENOMEM);
	return map_object(object, prot, {cur_region->end() - object->size(), object->size()});
}

Result VMSpace::unmap_region(VMRegion& region) {
	m_lock.acquire();
	VMSpaceRegion* cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->vmRegion == &region) {
			if(cur_region->vmRegion) {
				cur_region->vmRegion->m_space.reset();
				m_page_directory.unmap(*cur_region->vmRegion);
				m_lock.release();
				auto free_res = free_region(cur_region);
				ASSERT(!free_res.is_error());
				return free_res;
			}
			m_lock.release();
			return Result(ENOENT);
		}
		cur_region = cur_region->next;
	}
	m_lock.release();
	return Result(ENOENT);
}

Result VMSpace::unmap_region(VirtualAddress address) {
	m_lock.acquire();
	VMSpaceRegion* cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->start == address) {
			if(cur_region->vmRegion) {
				cur_region->vmRegion->m_space.reset();
				m_page_directory.unmap(*cur_region->vmRegion);
				m_lock.release();
				auto free_res = free_region(cur_region);
				ASSERT(!free_res.is_error());
				return free_res;
			}
			m_lock.release();
			return Result(ENOENT);
		}
		cur_region = cur_region->next;
	}
	m_lock.release();
	return Result(ENOENT);
}

ResultRet<kstd::Arc<VMRegion>> VMSpace::get_region_at(VirtualAddress address) {
	LOCK(m_lock);
	VMSpaceRegion* cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->start == address) {
			if(cur_region->vmRegion)
				return cur_region->vmRegion->self();
			return Result(ENOENT);
		}
		cur_region = cur_region->next;
	}
	return Result(ENOENT);
}

ResultRet<kstd::Arc<VMRegion>> VMSpace::get_region_containing(VirtualAddress address) {
	LOCK(m_lock);
	VMSpaceRegion* cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->contains(address)) {
			if(cur_region->vmRegion)
				return cur_region->vmRegion->self();
			return Result(ENOENT);
		}
		cur_region = cur_region->next;
	}
	return Result(ENOENT);
}

Result VMSpace::reserve_region(VirtualAddress start, size_t size) {
	LOCK(m_lock);
	return alloc_space_at(size, start).result();
}

Result VMSpace::try_pagefault(PageFault fault) {
	LOCK(m_lock);
	auto cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->contains(fault.address)) {
			auto vmRegion = cur_region->vmRegion;
			if(!vmRegion)
				return Result(EINVAL);

			// If this region has a sentinel page, then we might be within the VMSpaceRegion but not the vmRegion.
			if (!vmRegion->contains(fault.address))
				return Result(ENOENT);

			// First, sanity check. If the region doesn't have the proper permissions, we can just fail here.
			auto prot = vmRegion->prot();
			if(
				(!prot.read && fault.type == PageFault::Type::Read) ||
				(!prot.write && fault.type == PageFault::Type::Write) ||
				(!prot.execute && fault.type == PageFault::Type::Execute)
			) {
				return Result(EINVAL);
			}

			PageIndex error_page = (fault.address - vmRegion->start()) / PAGE_SIZE;
			PageIndex object_page = error_page + (vmRegion->object_start() / PAGE_SIZE);
			auto object = vmRegion->object();

			// Check to see if it needs to be read in
			LOCK_N(object->lock(), object_locker);
			if(object->physical_page_index(object_page)) {
				// This page may be marked CoW, so copy it if it is
				if(vmRegion->prot().write && object->page_is_cow(object_page)) {
					auto res = vmRegion->m_object->try_cow_page(object_page);
					if(res.is_error())
						return res;
				}

				// Or, we may have encountered a race where the page was created by another thread after the fault.
				m_page_directory.map(*vmRegion, VirtualRange { object_page * PAGE_SIZE, PAGE_SIZE });
				return Result(SUCCESS);
			}

			// Otherwise, read in the page and map it
			auto did_read = TRY(object->try_fault_in_page(object_page));
			ASSERT(object->physical_page_index(object_page));
			if(did_read)
				m_page_directory.map(*vmRegion, VirtualRange { error_page * PAGE_SIZE, PAGE_SIZE });

			return Result(SUCCESS);
		}
		cur_region = cur_region->next;
	}

	return Result(ENOENT);
}

ResultRet<VirtualAddress> VMSpace::find_free_space(size_t size) {
	LOCK(m_lock);
	auto cur_region = m_region_map;
	while(cur_region) {
		if(!cur_region->used && cur_region->size >= size)
			return cur_region->start;
		cur_region = cur_region->next;
	}
	return Result(ENOMEM);
}

size_t VMSpace::calculate_regular_anonymous_total() {
	LOCK(m_lock);
	size_t total_pages = 0;
	auto cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->used) {
			auto object = cur_region->vmRegion->m_object;
			total_pages += object->num_committed_pages();
		}
		cur_region = cur_region->next;
	}
	return total_pages * PAGE_SIZE;
}

void VMSpace::iterate_regions(kstd::IterationFunc<VMRegion*> callback) {
	LOCK(m_lock);
	auto cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->used)
			ITER_BREAK(callback(cur_region->vmRegion));
		cur_region = cur_region->next;
	}
}

ResultRet<VMSpace::VMSpaceRegion*> VMSpace::alloc_space(size_t size) {
	ASSERT(size % PAGE_SIZE == 0);

	/**
	 * We allocate a new region if we need one BEFORE iterating through the regions, because there's a chance we'll
	 * need to allocate more pages for the heap and if we're in the middle of iterating through regions when that
	 * happens, it could get ugly.
	 */
	auto new_region = new VMSpaceRegion;

	{
		LOCK(m_lock);
		auto cur_region = m_region_map;
		while(cur_region) {
			if(cur_region->used || cur_region->size < size) {
				cur_region = cur_region->next;
				continue;
			}

			if(cur_region->size == size) {
				cur_region->used = true;
				m_used += cur_region->size;
				delete new_region;
				return cur_region;
			}

			*new_region = VMSpaceRegion {
					.start = cur_region->start,
					.size = size,
					.used = true,
					.next = cur_region,
					.prev = cur_region->prev
			};

			if(cur_region->prev)
				cur_region->prev->next = new_region;

			cur_region->start += size;
			cur_region->size -= size;
			cur_region->prev = new_region;
			m_used += new_region->size;

			if(m_region_map == cur_region)
				m_region_map = new_region;
			return new_region;
		}
	}

	delete new_region;
	return Result(ENOMEM);
}

ResultRet<VMSpace::VMSpaceRegion*> VMSpace::alloc_space_at(size_t size, VirtualAddress address) {
	ASSERT(address % PAGE_SIZE == 0);
	ASSERT(size % PAGE_SIZE == 0);

	/**
	 * We allocate new regions if we need one BEFORE iterating through the regions, because there's a chance we'll
	 * need to allocate more pages for the heap and if we're in the middle of iterating through regions when that
	 * happens, it could get ugly.
	 */
	auto new_region_before = new VMSpaceRegion;
	auto new_region_after = new VMSpaceRegion;

	{
		LOCK(m_lock);
		auto cur_region = m_region_map;
		while(cur_region) {
			if(cur_region->contains(address)) {
				if(cur_region->used) {
					delete new_region_before;
					delete new_region_after;
					return Result(ENOMEM);
				}

				if(cur_region->size == size) {
					cur_region->used = true;
					m_used += cur_region->size;
					delete new_region_before;
					delete new_region_after;
					return cur_region;
				}

				if(cur_region->size - (address - cur_region->start) >= size) {
					// Create new region before if needed
					if(cur_region->start < address) {
						*new_region_before = VMSpaceRegion {
								.start = cur_region->start,
								.size = address - cur_region->start,
								.used = false,
								.next = cur_region,
								.prev = cur_region->prev
						};
						if(cur_region->prev)
							cur_region->prev->next = new_region_before;
						cur_region->prev = new_region_before;
						if(m_region_map == cur_region)
							m_region_map = new_region_before;
					} else {
						delete new_region_before;
					}

					// Create new region after if needed
					if(cur_region->end() > address + size) {
						*new_region_after = VMSpaceRegion {
								.start = address + size,
								.size = cur_region->end() - (address + size),
								.used = false,
								.next = cur_region->next,
								.prev = cur_region
						};
						if(cur_region->next)
							cur_region->next->prev = new_region_after;
						cur_region->next = new_region_after;
					} else {
						delete new_region_after;
					}

					cur_region->start = address;
					cur_region->size = size;
					cur_region->used = true;
					m_used += cur_region->size;
					return cur_region;
				}

				return Result(ENOMEM);
			}

			cur_region = cur_region->next;
		}
	}

	return Result(ENOMEM);
}

Result VMSpace::free_region(VMSpaceRegion* region) {
	VMSpaceRegion* to_delete[2] = {nullptr, nullptr};
	{
		LOCK(m_lock);
		region->used = false;
		region->vmRegion = nullptr;
		m_used -= region->size;

		// Merge previous region if needed
		if(region->prev && !region->prev->used) {
			to_delete[0] = region->prev;
			region->prev = region->prev->prev;
			if(to_delete[0]->prev)
				to_delete[0]->prev->next = region;
			region->start -= to_delete[0]->size;
			region->size += to_delete[0]->size;
			if(m_region_map == to_delete[0])
				m_region_map = region;
		}

		// Merge next region if needed
		if(region->next && !region->next->used) {
			to_delete[1] = region->next;
			region->next = region->next->next;
			if(to_delete[1]->next)
				to_delete[1]->next->prev = region;
			region->size += to_delete[1]->size;
		}
	}

	// We do this while not holding the lock just in case this triggers a page free in the allocator.
	delete to_delete[0];
	delete to_delete[1];

	return Result(SUCCESS);
}
