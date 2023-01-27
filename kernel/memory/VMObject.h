/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../kstd/vector.hpp"
#include "PhysicalPage.h"
#include "../kstd/Arc.h"
#include "../Result.hpp"
#include "../api/errno.h"

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

	explicit VMObject(kstd::vector<PageIndex> physical_pages);
	VMObject(const VMObject& other) = delete;
	virtual ~VMObject();

	virtual bool is_anonymous() const { return false; }
	virtual bool is_inode() const { return false; }

	size_t size() const { return m_size; }
	virtual PhysicalPage& physical_page(size_t index) const;
	virtual ForkAction fork_action() const { return ForkAction::Share; }
	virtual ResultRet<kstd::Arc<VMObject>> copy_on_write() { return Result(EINVAL); }

protected:
	kstd::vector<PageIndex> m_physical_pages;
	size_t m_size;
};
