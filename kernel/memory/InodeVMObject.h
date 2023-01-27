/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "VMObject.h"
#include "../filesystem/Inode.h"

class InodeVMObject: public VMObject {
public:
	static kstd::Arc<InodeVMObject> make_for_inode(kstd::Arc<Inode> inode);

	bool is_inode() const override { return true; }

	PageIndex& physical_page_index(size_t index) const {
		return m_physical_pages[index];
	};

	kstd::Weak<Inode> inode() const { return m_inode; }
	SpinLock& lock() { return m_lock; }

	// TODO: Syncing

private:
	explicit InodeVMObject(kstd::Arc<Inode> inode, kstd::vector<PageIndex> physical_pages);

	kstd::Weak<Inode> m_inode;
	SpinLock m_lock;
};
