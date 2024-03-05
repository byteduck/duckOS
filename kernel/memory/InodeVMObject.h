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

	static kstd::Arc<InodeVMObject> make_for_inode(kstd::string name, kstd::Arc<Inode> inode, Type type);

	/**
	 * Reads in the page at the given index if it isn't allocated yet.
	 * @param index The index of the page to read in.
	 * @return A successful result if the index is in range and could be read. True if read, false if already exists.
	 */
	ResultRet<bool> try_fault_in_page(PageIndex index) override;

	kstd::Arc<Inode> inode() const { return m_inode; }
	Type type() const { return m_type; }
	bool is_inode() const override { return true; }
	ForkAction fork_action() const override {
		return m_type == Type::Private ? ForkAction::BecomeCoW : ForkAction::Share;
	}
	ResultRet<kstd::Arc<VMObject>> clone() override;
	size_t num_committed_pages() const override { return m_committed_pages; }

	// TODO: Syncing

private:
	explicit InodeVMObject(kstd::string name, kstd::vector<PageIndex> physical_pages, kstd::Arc<Inode> inode, Type type, bool cow);

	kstd::Arc<Inode> m_inode;
	Type m_type;
	size_t m_committed_pages = 0;
};
