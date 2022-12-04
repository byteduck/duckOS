/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../kstd/vector.hpp"
#include "PhysicalPage.h"
#include "../kstd/Arc.h"

/**
 * This is a base class to describe a (contiguous) object in virtual memory. This object may be shared across multiple
 * address spaces (ie page directories / processes), and may be mapped at different virtual locations in each one.
 * VMObject is never used directly - instead, its subclasses are used.
 */
class VMObject: public kstd::ArcSelf<VMObject> {
public:
	explicit VMObject(kstd::vector<PageIndex> physical_pages);
	VMObject(const VMObject& other) = delete;
	virtual ~VMObject();

	virtual bool is_anonymous() const { return false; }

	size_t size() const { return m_size; }
	PhysicalPage& physical_page(size_t index) const;

private:
	kstd::vector<PageIndex> m_physical_pages;
	size_t m_size;
};
