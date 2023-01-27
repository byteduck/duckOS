/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "InodeVMObject.h"

kstd::Arc<InodeVMObject> InodeVMObject::make_for_inode(kstd::Arc<Inode> inode, InodeVMObject::Type type) {
	kstd::vector<PageIndex> pages;
	pages.resize((inode->metadata().size + PAGE_SIZE - 1) / PAGE_SIZE);
	memset(pages.storage(), 0, pages.size() * sizeof(PageIndex));
	return kstd::Arc<InodeVMObject>(new InodeVMObject(pages, kstd::move(inode), type));
}

ResultRet<kstd::Arc<VMObject>> InodeVMObject::copy_on_write() {
	ASSERT(m_inode);

	// Copy the pages that are allocated
	kstd::vector<PageIndex> new_pages;
	new_pages.reserve(m_physical_pages.size());
	for(auto page : m_physical_pages) {
		if(!page) {
			new_pages.push_back(0);
			continue;
		}
		auto new_page_res = MM.alloc_physical_page();
		ASSERT(!new_page_res.is_error());
		new_pages.push_back(new_page_res.value());

		MM.with_dual_quickmapped(page, new_page_res.value(), [&] (void* old_page, void* new_page) {
			memcpy(new_page, old_page, PAGE_SIZE);
		});
	}

	// Create the new object with the pages
	auto new_object = kstd::Arc<InodeVMObject>(new InodeVMObject(new_pages, m_inode.lock(), m_type));
	return kstd::static_pointer_cast<VMObject>(new_object);
}

InodeVMObject::InodeVMObject( kstd::vector<PageIndex> physical_pages, kstd::Arc<Inode> inode, InodeVMObject::Type type):
	VMObject(kstd::move(physical_pages)),
	m_inode(kstd::move(inode)),
	m_type(type)
{}
