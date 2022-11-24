/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "VMSpace.h"
#include "MemoryManager.h"
#include "AnonymousVMObject.h"
#include "../kstd/cstring.h"

const VMProt VMSpace::default_prot = {
	.read = true,
	.write = true,
	.execute = true,
	.cow = false
};

VMSpace::VMSpace(VirtualAddress start, size_t size, PageDirectory& page_directory):
	m_start(start),
	m_size(size),
	m_region_map(new VMSpaceRegion {.start = start, .size = size, .used = false, .next = nullptr, .prev = nullptr}),
	m_page_directory(page_directory)
{}

VMSpace::~VMSpace() {
	// We have to remove the pointer to this in all child regions since we don't want them calling free_region on this
	for(size_t i = 0; i < m_regions.size(); i++)
		m_regions[i]->m_space = nullptr;

	auto cur_region = m_region_map;
	while(cur_region) {
		auto next = cur_region->next;
		delete cur_region;
		cur_region = next;
	}
}

Ptr<VMSpace> VMSpace::fork(PageDirectory& page_directory, kstd::vector<Ptr<VMRegion>>& regions_vec) {
	LOCK(m_lock);
	auto new_space = new VMSpace(m_start, m_size, page_directory);
	new_space->m_used = m_used;
	delete new_space->m_region_map;

	// Clone allocation regions
	auto cur_region = m_region_map;
	VMSpaceRegion* prev_new_region = nullptr;
	while(cur_region) {
		auto new_region = new VMSpaceRegion(*cur_region);
		if(cur_region == m_region_map)
			new_space->m_region_map = new_region;
		new_region->prev = prev_new_region;
		if(prev_new_region)
			prev_new_region->next = new_region;
		prev_new_region = new_region;
		cur_region = cur_region->next;
	}

	// Copy regions / objects
	new_space->m_regions.reserve(m_regions.size());
	regions_vec.reserve(regions_vec.size() + m_regions.size());
	for(size_t i = 0; i < m_regions.size(); i++) {
		auto& region = m_regions[i];

		if(region->object()->is_anonymous()) {
			// Remap anonymous writeable regions as CoW
			if(region->prot().write) {
				region->set_cow(true);
				m_page_directory.map(*region);
			}

			// Map into new space
			auto new_region = new VMRegion(region->object(), new_space, region->start(), region->size(), region->prot());
			page_directory.map(*new_region);
			new_space->m_regions.push_back(new_region);
			regions_vec.push_back(Ptr<VMRegion>(new_region));
		} else {
			ASSERT(false);
		}
	}

	return Ptr<VMSpace>(new_space);
}

ResultRet<Ptr<VMRegion>> VMSpace::map_object(Ptr<VMObject> object, VMProt prot) {
	LOCK(m_lock);
	auto vaddr = TRY(alloc_space(object->size()));
	auto region = kstd::make_shared<VMRegion>(
			object,
			this,
			vaddr, object->size(),
			prot);
	m_regions.push_back(region.get());
	m_page_directory.map(*region);
	return region;
}

ResultRet<Ptr<VMRegion>> VMSpace::map_object(Ptr<VMObject> object, VirtualAddress address, VMProt prot) {
	LOCK(m_lock);
	auto vaddr = TRY(alloc_space_at(object->size(), address));
	auto region = kstd::make_shared<VMRegion>(
			object,
			this,
			vaddr,
			object->size(),
			prot);
	m_regions.push_back(region.get());
	m_page_directory.map(*region);
	return region;
}

Result VMSpace::unmap_region(VMRegion& region) {
	LOCK(m_lock);
	for(size_t i = 0; i < m_regions.size(); i++) {
		if(m_regions[i] == &region) {
			region.m_space = nullptr;
			m_regions.erase(i);
			auto free_res = free_space(region.size(), region.start());
			ASSERT(!free_res.is_error())
			m_page_directory.unmap(region);
			return free_res;
		}
	}
	return Result(ENOENT);
}

Result VMSpace::unmap_region(VirtualAddress address) {
	LOCK(m_lock);
	for(size_t i = 0; i < m_regions.size(); i++) {
		auto region = m_regions[i];
		if(region->start() == address) {
			region->m_space = nullptr;
			auto free_res = free_space(region->size(), region->start());
			ASSERT(!free_res.is_error())
			m_page_directory.unmap(*region);
			m_regions.erase(i);
			return free_res;
		}
	}
	return Result(ENOENT);
}

ResultRet<VMRegion*> VMSpace::get_region(VirtualAddress address) {
	LOCK(m_lock);
	for(size_t i = 0; i < m_regions.size(); i++) {
		auto region = m_regions[i];
		if(region->start() == address)
			return region;
	}
	return Result(ENOENT);
}

Result VMSpace::reserve_region(VirtualAddress start, size_t size) {
	LOCK(m_lock);
	return alloc_space_at(size, start).result();
}

Result VMSpace::try_pagefault(VirtualAddress error_pos) {
	LOCK(m_lock);

	for(size_t i = 0; i < m_regions.size(); i++) {
		auto& region = m_regions[i];
		if(region->contains(error_pos)) {
			// Check if the region is CoW.
			if(region->is_cow() && region->object()->is_anonymous()) {
				// TODO: Check if we're mapped first? We should be.
				auto new_object = TRY(AnonymousVMObject::alloc(region->object()->size()));
				auto mapped_object = MM.map_object(new_object);
				memcpy((void*) mapped_object->start(), (void*) region->start(), new_object->size());
				region->m_object = new_object;
				region->set_cow(false);
				m_page_directory.map(*region);
				return Result(SUCCESS);
			}

			return Result(EINVAL);
		}
	}

	return Result(ENOENT);
}

ResultRet<VirtualAddress> VMSpace::alloc_space(size_t size) {
	ASSERT(size % PAGE_SIZE == 0);
	auto cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->used) {
			cur_region = cur_region->next;
			continue;
		}

		if(cur_region->size == size) {
			cur_region->used = true;
			m_used += cur_region->size;
			return cur_region->start;
		}

		if(cur_region->size >= size) {
			auto new_region = new VMSpaceRegion {
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

			return new_region->start;
		}

		cur_region = cur_region->next;
	}

	return Result(ENOMEM);
}

ResultRet<VirtualAddress> VMSpace::alloc_space_at(size_t size, VirtualAddress address) {
	ASSERT(address % PAGE_SIZE == 0);
	ASSERT(size % PAGE_SIZE == 0);
	auto cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->contains(address)) {
			if(cur_region->used)
				return Result(ENOMEM);

			if(cur_region->size == size) {
				cur_region->used = true;
				m_used += cur_region->size;
				return cur_region->start;
			}

			if(cur_region->size - (address - cur_region->start) >= size) {
				// Create new region before if needed
				if(cur_region->start < address) {
					auto new_region = new VMSpaceRegion {
						.start = cur_region->start,
						.size = address - cur_region->start,
						.used = false,
						.next = cur_region,
						.prev = cur_region->prev
					};
					if(cur_region->prev)
						cur_region->prev->next = new_region;
					cur_region->prev = new_region;
					if(m_region_map == cur_region)
						m_region_map = new_region;
				}

				// Create new region after if needed
				if(cur_region->end() > address + size) {
					auto new_region = new VMSpaceRegion {
						.start = address + size,
						.size = cur_region->end() - (address + size),
						.used = false,
						.next = cur_region->next,
						.prev = cur_region
					};
					if(cur_region->next)
						cur_region->next->prev = new_region;
					cur_region->next = new_region;
				}

				cur_region->start = address;
				cur_region->size = size;
				cur_region->used = true;
				m_used += cur_region->size;
				return address;
			}

			return Result(ENOMEM);
		}

		cur_region = cur_region->next;
	}

	return Result(ENOMEM);
}

Result VMSpace::free_space(size_t size, VirtualAddress address) {
	ASSERT(size % PAGE_SIZE == 0);
	ASSERT(address % PAGE_SIZE == 0);
	auto cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->start == address) {
			ASSERT(cur_region->size == size);
			cur_region->used = false;

			// Merge previous region if needed
			if(cur_region->prev && !cur_region->prev->used) {
				auto to_delete = cur_region->prev;
				cur_region->prev = cur_region->prev->prev;
				if(to_delete->prev)
					to_delete->prev->next = cur_region;
				cur_region->start -= to_delete->size;
				cur_region->size += to_delete->size;
				if(m_region_map == to_delete)
					m_region_map = cur_region;
				delete to_delete;
			}

			// Merge next region if needed
			if(cur_region->next && !cur_region->next->used) {
				auto to_delete = cur_region->next;
				cur_region->next = cur_region->next->next;
				if(to_delete->next)
					to_delete->next->prev = cur_region;
				cur_region->size += to_delete->size;
				delete to_delete;
			}

			m_used -= size;
			return Result(SUCCESS);
		}
		cur_region = cur_region->next;
	}
	ASSERT(false);
}
