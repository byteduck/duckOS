/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../kstd/unix_types.h"
#include "BuddyZone.h"
#include "../kstd/vector.hpp"
#include "../tasking/SpinLock.h"

class PhysicalRegion {
public:
	PhysicalRegion(size_t start_page, size_t num_pages, bool reserved, bool used);
	~PhysicalRegion();

	size_t start_page() const { return m_start_page; }
	size_t num_pages() const { return m_num_pages; }
	size_t free_pages() const { return m_free_pages; }
	bool reserved() const { return m_reserved; }

	ResultRet<PageIndex> alloc_page();
	void free_page(PageIndex page);

protected:
	friend class MemoryManager;
	void init();

private:
	SpinLock m_lock;
	kstd::vector<BuddyZone*> m_zones;
	PageIndex m_start_page;
	size_t m_num_pages;
	size_t m_free_pages;
	const bool m_reserved;
};
