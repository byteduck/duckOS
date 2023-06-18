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

	/**
	 * Reads in the page at the given index if it isn't allocated yet.
	 * @param index The index of the page to read in.
	 * @return A successful result if the index is in range and could be read. True if read, false if already exists.
	 */
	ResultRet<bool> read_page_if_needed(size_t index);

	/**
	 * Maps the given range of pages into the kernel, reading them in as needed.
	 * @param start_page The page to start the mapping at.
	 * @param num_pages The number of pages to map.
	 */
	ResultRet<kstd::Arc<VMRegion>> map_pages_in_kernel(PageIndex start_page, size_t num_pages);

	kstd::Arc<Inode> inode() const { return m_inode; }
	SpinLock& lock() { return m_page_lock; }
	Type type() const { return m_type; }
	bool is_inode() const override { return true; }
	ForkAction fork_action() const override {
		return m_type == Type::Private ? ForkAction::BecomeCoW : ForkAction::Share;
	}
	ResultRet<kstd::Arc<VMObject>> clone() override;

	// TODO: Syncing

private:
	explicit InodeVMObject(kstd::vector<PageIndex> physical_pages, kstd::Arc<Inode> inode, Type type, bool cow);

	kstd::Arc<Inode> m_inode;
	Type m_type;
};
