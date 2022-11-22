/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "Memory.h"
#include "VMObject.h"
#include "../kstd/shared_ptr.hpp"

/**
 * This class describes a region in virtual memory in a specific address space.
 */
class VMRegion {
public:
	struct Prot {
		bool read : 1;
		bool write : 1;
		bool execute : 1;
	};

	/**
	 * Creates a new virtual memory region.
	 * @param object The VMObject that this region corresponds to.
	 * @param start The start address of the region.
	 * @param size The end address of the region.
	 */
	VMRegion(kstd::shared_ptr<VMObject> object, size_t start, size_t size);
	~VMRegion();

	kstd::shared_ptr<VMObject> object() const { return m_object; }
	VirtualAddress start() const { return m_start; }
	VirtualAddress end() const { return m_start + m_size; }
	size_t size() const { return m_size; }
	bool is_kernel() const { return m_start >= HIGHER_HALF; }
	Prot prot() const { return m_prot; }

private:

	kstd::shared_ptr<VMObject> m_object;
	size_t m_start;
	size_t m_size;
	Prot m_prot;
};
