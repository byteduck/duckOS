/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "Memory.h"
#include "VMObject.h"
#include "../kstd/Arc.h"

struct VMProt {
	static VMProt RWX;
	static VMProt RW;
	static VMProt RX;
	static VMProt R;
	bool read : 1;
	bool write : 1;
	bool execute : 1;

	constexpr bool operator==(VMProt other) {
		return other.read == read && other.write == write && other.execute == execute;
	}
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
	VMRegion(kstd::Arc<VMObject> object, kstd::Arc<VMSpace> space, VirtualRange range, VirtualAddress object_start, VMProt prot);
	~VMRegion();

	kstd::Arc<VMObject> object() const { return m_object; }
	VirtualAddress start() const { return m_range.start; }
	VirtualRange range() const { return m_range; }
	VirtualAddress object_start() const { return m_object_start; }
	VirtualAddress end() const { return m_range.end(); }
	size_t size() const { return m_range.size; }
	bool is_kernel() const { return m_range.start >= HIGHER_HALF; }
	bool contains(VirtualAddress address) const { return m_range.contains(address); }
	VMProt prot() const { return m_prot; }
	void set_prot(VMProt prot);

private:
	friend class VMSpace;
	kstd::Arc<VMObject> m_object; /// The object this VMRegion is associated with.
	kstd::Weak<VMSpace> m_space; /// The VMSpace this region belongs to.
	VirtualRange m_range; /// Where in the VMSpace this region resides.
	size_t m_object_start; /// Where in the VMObject this region begins.
	VMProt m_prot; /// The protection of this region.
};
