/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "VMObject.h"
#include "MemoryManager.h"

VMObject::VMObject(kstd::vector<PageIndex> physical_pages):
	m_physical_pages(kstd::move(physical_pages)),
	m_size(m_physical_pages.size() * PAGE_SIZE)
{

}

VMObject::~VMObject() {
	for(size_t i = 0; i < m_physical_pages.size(); i++)
		MemoryManager::inst().get_physical_page(m_physical_pages[i]).unref();
}

PhysicalPage& VMObject::physical_page(size_t index) const {
	return MemoryManager::inst().get_physical_page(m_physical_pages[index]);
}
