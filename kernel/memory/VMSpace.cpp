/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "VMSpace.h"
#include "../kstd/unix_types.h"

VMSpace::VMSpace(VirtualAddress start, size_t size):
	m_start(start),
	m_size(size),
	m_region_map(new VMSpaceRegion {.start = start, .size = size, .used = false, .next = nullptr, .prev = nullptr}) {}

VMSpace::~VMSpace() {
	auto cur_region = m_region_map;
	while(cur_region) {
		auto next = cur_region->next;
		delete cur_region;
		cur_region = next;
	}
}

ResultRet<kstd::shared_ptr<VMRegion>> VMSpace::map_object(kstd::shared_ptr<VMObject> object) {
	LOCK(m_lock);
	auto vaddr = TRY(alloc_space(object->size()));
	auto region = kstd::make_shared<VMRegion>(object, vaddr, object->size());
	m_regions.push_back(region);
	m_page_directory.map(region);
	return region;
}

ResultRet<kstd::shared_ptr<VMRegion>> VMSpace::map_object(kstd::shared_ptr<VMObject> object, VirtualAddress address) {
	LOCK(m_lock);
	auto vaddr = TRY(alloc_space_at(object->size(), address));
	auto region = kstd::make_shared<VMRegion>(object, vaddr, object->size());
	m_regions.push_back(region);
	m_page_directory.map(region);
	return region;
}

Result VMSpace::unmap_region(kstd::shared_ptr<VMRegion> region) {
	LOCK(m_lock);
	for(size_t i = 0; i < m_regions.size(); i++) {
		if(m_regions[i] == region) {
			m_regions.erase(i);
			auto free_res = free_space(region->size(), region->start());
			ASSERT(!free_res.is_error())
			m_page_directory.unmap(region);
			return free_res;
		}
	}
	return Result(ENOENT);
}

ResultRet<VirtualAddress> VMSpace::alloc_space(size_t size) {
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
			return new_region->start;
		}

		cur_region = cur_region->next;
	}
	return Result(ENOMEM);
}

ResultRet<VirtualAddress> VMSpace::alloc_space_at(size_t size, VirtualAddress address) {
	auto cur_region = m_region_map;
	while(cur_region) {
		if(cur_region->contains(address, size)) {
			if(cur_region->used)
				return Result(ENOMEM);

			if(cur_region->size == size) {
				cur_region->used = true;
				m_used += cur_region->size;
				return cur_region->start;
			}

			if(cur_region->size >= size) {
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
				delete to_delete;
			}

			// Merge next region if needed
			if(cur_region->next && !cur_region->next->used) {
				auto to_delete = cur_region->next;
				cur_region->next = cur_region->next->next;
				if(to_delete->next)
					to_delete->next->prev = cur_region;
				delete to_delete;
			}

			m_used -= size;
			return Result(SUCCESS);
		}
		cur_region = cur_region->next;
	}
	ASSERT(false);
}
