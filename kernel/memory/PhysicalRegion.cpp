/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "PhysicalRegion.h"
#include "MemoryManager.h"

PhysicalRegion::PhysicalRegion(size_t start_page, size_t num_pages, bool reserved, bool used):
	m_start_page(start_page),
	m_num_pages(num_pages),
	m_free_pages(used ? 0 : num_pages),
	m_reserved(reserved)
{
	// If the region is reserved, don't bother allocating zones
	if(reserved)
		return;

	// Create zones to fill the region
	while(num_pages) {
		// Round down num_pages to a power of 2, and create a zone with that many pages (or MAX_ORDER, whichever is smaller)
		size_t zone_order = (sizeof(unsigned int) * 8) - __builtin_clz(num_pages) - 1;
		if(zone_order > BuddyZone::MAX_ORDER)
			zone_order = BuddyZone::MAX_ORDER;
		size_t zone_num_pages = 1 << zone_order;
		auto zone = new BuddyZone(start_page, zone_num_pages);
		m_zones.push_back(zone);
		num_pages -= zone_num_pages;
		start_page += zone_num_pages;

		// If this region is used, mark the entire zone used too
		if(used) {
			auto res = zone->alloc_block(zone_num_pages);
			ASSERT(!res.is_error());
		}
	}
}

PhysicalRegion::~PhysicalRegion() {
	for(size_t zone = 0; zone < m_zones.size(); zone++)
		delete m_zones[zone];
}

ResultRet<PageIndex> PhysicalRegion::alloc_page() {
	if(m_reserved)
		return Result(ENOMEM);

	LOCK(m_lock);
	if(!m_free_pages)
		return Result(ENOMEM);

	for(size_t zone = 0; zone < m_zones.size(); zone++) {
		auto page_res = m_zones[zone]->alloc_block(1);
		if(!page_res.is_error()) {
			m_free_pages--;
			return page_res.value();
		}
	}

	return Result(ENOMEM);
}

ResultRet<PageIndex> PhysicalRegion::alloc_pages(size_t num_pages) {
	if(m_reserved)
		return Result(ENOMEM);

	LOCK(m_lock);
	if(m_free_pages < num_pages)
		return Result(ENOMEM);

	auto order = BuddyZone::order_for(num_pages);
	if(order > BuddyZone::MAX_ORDER)
		return Result(EINVAL);

	for(size_t zone = 0; zone < m_zones.size(); zone++) {
		auto page_res = m_zones[zone]->alloc_block(num_pages);
		if(!page_res.is_error()) {
			m_free_pages -= num_pages;
			return page_res.value();
		}
	}

	return Result(ENOMEM);
}

void PhysicalRegion::free_page(PageIndex page) {
	ASSERT(!m_reserved);
	ASSERT(page >= m_start_page && page < m_start_page + m_num_pages);
	LOCK(m_lock);
	for(size_t i = 0; i < m_zones.size(); i++) {
		auto zone = m_zones[i];
		if(zone->contains_page(page)) {
			m_free_pages++;
			zone->free_block(page, 1);
			return;
		}
	}
	ASSERT(false);
}

bool PhysicalRegion::contains_page(PageIndex page) {
	return page >= m_start_page && page < m_start_page + m_num_pages;
}

void PhysicalRegion::init() {
	if(m_reserved)
		return;
	// Setup the freelists of the zones.
	for(auto& zone : m_zones) {
		auto& page = MemoryManager::inst().get_physical_page(zone->first_page());
		page.free.prev = -1;
		page.free.next = -1;
	}
}
