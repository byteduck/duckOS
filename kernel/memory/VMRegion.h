/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "Memory.h"
#include "VMObject.h"
#include "../kstd/Arc.h"

struct VMProt {
	bool read : 1;
	bool write : 1;
	bool execute : 1;
	bool cow : 1;
};

class VMSpace;

/**
 * This class describes a region in virtual memory in a specific address space.
 */
class VMRegion: public kstd::ArcSelf<VMRegion> {
public:
	/**
	 * Creates a new virtual memory region.
	 * @param object The VMObject that this region corresponds to.
	 * @param start The start address of the region.
	 * @param size The end address of the region.
	 */
	VMRegion(kstd::Arc<VMObject> object, kstd::Arc<VMSpace> space, size_t start, size_t size, VMProt prot);
	~VMRegion();

	kstd::Arc<VMObject> object() const { return m_object; }
	VirtualAddress start() const { return m_start; }
	VirtualAddress end() const { return m_start + m_size; }
	size_t size() const { return m_size; }
	bool is_kernel() const { return m_start >= HIGHER_HALF; }
	bool is_cow() const { return m_prot.cow; }
	bool contains(VirtualAddress address) const { return address >= m_start && address < end(); }
	VMProt prot() const { return m_prot; }

	void set_cow(bool cow);

private:
	friend class VMSpace;
	kstd::Arc<VMObject> m_object;
	kstd::Weak<VMSpace> m_space;
	size_t m_start;
	size_t m_size;
	VMProt m_prot;
};
