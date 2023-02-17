/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "InodeVMObject.h"

kstd::Arc<InodeVMObject> InodeVMObject::make_for_inode(kstd::Arc<Inode> inode, InodeVMObject::Type type) {
	kstd::vector<PageIndex> pages;
	pages.resize((inode->metadata().size + PAGE_SIZE - 1) / PAGE_SIZE);
	memset(pages.storage(), 0, pages.size() * sizeof(PageIndex));
	return kstd::Arc<InodeVMObject>(new InodeVMObject(pages, kstd::move(inode), type, false));
}

ResultRet<kstd::Arc<VMObject>> InodeVMObject::clone() {
	ASSERT(m_inode);
	LOCK(m_page_lock);
	ASSERT(m_type == Type::Private);
	become_cow_and_ref_pages();
	auto new_object = kstd::Arc(new InodeVMObject(m_physical_pages, m_inode, m_type, m_type == Type::Private));
	return kstd::static_pointer_cast<VMObject>(new_object);
}

InodeVMObject::InodeVMObject(kstd::vector<PageIndex> physical_pages, kstd::Arc<Inode> inode, InodeVMObject::Type type, bool cow):
	VMObject(kstd::move(physical_pages), cow),
	m_inode(kstd::move(inode)),
	m_type(type)
{}
