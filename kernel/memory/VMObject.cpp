/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "VMObject.h"
#include "MemoryManager.h"

VMObject::VMObject(kstd::string name, kstd::vector<PageIndex> physical_pages, bool all_cow):
	m_name(kstd::move(name)),
	m_physical_pages(kstd::move(physical_pages)),
	m_cow_pages(m_physical_pages.size()),
	m_size(m_physical_pages.size() * PAGE_SIZE)
{
	if(all_cow) {
		for(size_t i = 0; i < m_physical_pages.size(); i++) {
			if(m_physical_pages[i])
				m_cow_pages.set(i, true);
		}
	}
}

VMObject::~VMObject() {
	for(auto physical_page : m_physical_pages)
		if(physical_page)
			MemoryManager::inst().get_physical_page(physical_page).unref();
}

PhysicalPage& VMObject::physical_page(size_t index) const {
	return MemoryManager::inst().get_physical_page(m_physical_pages[index]);
}

Result VMObject::try_cow_page(PageIndex page) {
	ASSERT(page < m_physical_pages.size());
	LOCK(m_page_lock);

	// If the page isn't CoW, don't proceed
	if(!page_is_cow(page))
		return Result(EINVAL);

	// Copy the page
	auto& old_page = m_physical_pages[page];
	ASSERT(old_page);
	auto new_page = TRY(MM.alloc_physical_page());
	MM.copy_page(old_page, new_page);

	// Unref the old page and replace it with the new one
	MM.get_physical_page(old_page).unref();
	old_page = new_page;

	// Mark the page not CoW
	m_cow_pages.set(page, false);

	return Result(Result::Success);
}

ResultRet<kstd::Arc<VMObject>> VMObject::clone() {
	ASSERT(false);
}

void VMObject::become_cow_and_ref_pages() {
	LOCK(m_page_lock);
	for(size_t i = 0; i < m_physical_pages.size(); i++) {
		if(m_physical_pages[i]) {
			m_cow_pages.set(i, true);
			physical_page(i).ref();
		}
	}
}
