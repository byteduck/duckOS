/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "VMObject.h"
#include "../filesystem/Inode.h"

class InodeVMObject: public VMObject {
public:
	enum class Type {
		Shared, Private
	};

	static kstd::Arc<InodeVMObject> make_for_inode(kstd::Arc<Inode> inode, Type type);


	PageIndex& physical_page_index(size_t index) const {
		return m_physical_pages[index];
	};

	kstd::Arc<Inode> inode() const { return m_inode; }
	SpinLock& lock() { return m_lock; }
	Type type() const { return m_type; }
	bool is_inode() const override { return true; }
	ForkAction fork_action() const override {
		return m_type == Type::Private ? ForkAction::BecomeCoW : ForkAction::Share;
	}
	ResultRet<kstd::Arc<VMObject>> copy_on_write() override;

	// TODO: Syncing

private:
	explicit InodeVMObject(kstd::vector<PageIndex> physical_pages, kstd::Arc<Inode> inode, Type type);

	kstd::Arc<Inode> m_inode;
	SpinLock m_lock;
	Type m_type;
};
