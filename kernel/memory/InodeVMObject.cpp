/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "InodeVMObject.h"

kstd::Arc<InodeVMObject> InodeVMObject::make_for_inode(kstd::Arc<Inode> inode) {
	kstd::vector<PageIndex> pages;
	pages.resize((inode->metadata().size + PAGE_SIZE - 1) / PAGE_SIZE);
	memset(pages.storage(), 0, pages.size() * sizeof(PageIndex));
	return kstd::Arc<InodeVMObject>(new InodeVMObject(inode, pages));
}

InodeVMObject::InodeVMObject(kstd::Arc<Inode> inode, kstd::vector<PageIndex> physical_pages):
	VMObject(physical_pages),
	m_inode(inode)
{}
