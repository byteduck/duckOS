/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../kstd/unix_types.h"
#include "BuddyZone.h"
#include "../kstd/vector.hpp"
#include "../tasking/Mutex.h"

class PhysicalRegion {
public:
	PhysicalRegion(size_t start_page, size_t num_pages, bool reserved, bool used);
	~PhysicalRegion();

	size_t start_page() const { return m_start_page; }
	size_t num_pages() const { return m_num_pages; }
	size_t free_pages() const { return m_free_pages; }
	bool reserved() const { return m_reserved; }

	/**
	 * Allocates a page in this region.
	 * @return The index of the page allocated (absolute).
	 */
	ResultRet<PageIndex> alloc_page();

	/**
	 * Allocates contiguous pages in this region.
	 * @return The index of the first page allocated (absolute).
	 */
	ResultRet<PageIndex> alloc_pages(size_t num_pages);

	/**
	 * Frees a page in this region.
	 * @param page The index of the page to free (absolute).
	 */
	void free_page(PageIndex page);

	/** Returns whether the given page is in this region. **/
	bool contains_page(PageIndex page);

protected:
	friend class MemoryManager;
	void init();

private:
	Mutex m_lock {"PhysicalRegion"};
	kstd::vector<BuddyZone*> m_zones;
	PageIndex m_start_page;
	size_t m_num_pages;
	size_t m_free_pages;
	const bool m_reserved;
};
