/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../kstd/vector.hpp"
#include "PhysicalPage.h"
#include "../kstd/Arc.h"
#include "../Result.hpp"
#include "../api/errno.h"
#include "../kstd/Bitmap.h"
#include "../tasking/Mutex.h"
#include <kernel/kstd/string.h>

/**
 * This is a base class to describe a (contiguous) object in virtual memory. This object may be shared across multiple
 * address spaces (ie page directories / processes), and may be mapped at different virtual locations in each one.
 * VMObject is never used directly - instead, its subclasses are used.
 */
class VMObject: public kstd::ArcSelf<VMObject> {
public:
	enum class ForkAction {
		BecomeCoW, Share, Ignore
	};

	explicit VMObject(kstd::string name,  kstd::vector<PageIndex> physical_pages, bool all_cow = false);
	VMObject(const VMObject& other) = delete;
	virtual ~VMObject();

	virtual bool is_anonymous() const { return false; }
	virtual bool is_inode() const { return false; }

	kstd::string name() const { return m_name; }
	size_t size() const { return m_size; }
	/** Gets the physical page at the given index in the object. **/
	virtual PhysicalPage& physical_page(size_t index) const;
	/** What the object should do when a memory space containing it is forked. **/
	virtual ForkAction fork_action() const { return ForkAction::Share; }

	/** Tries to copy the page at a given index if it is marked CoW. If it is not, EINVAL is returned. **/
	Result try_cow_page(PageIndex page);
	/** Returns whether a page in the object is marked CoW. **/
	bool page_is_cow(PageIndex page) const { return m_cow_pages.get(page); };
	/** Clones this VMObject using all the same physical pages and properties. **/
	virtual ResultRet<kstd::Arc<VMObject>> clone();

protected:
	/** Marks every page in this object as CoW, and increases the reference count of all pages by 1. **/
	void become_cow_and_ref_pages();

	kstd::string m_name;
	kstd::vector<PageIndex> m_physical_pages;
	kstd::Bitmap m_cow_pages;
	size_t m_size;
	Mutex m_page_lock {"VMObject::Page"};
};
