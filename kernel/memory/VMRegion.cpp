/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "VMRegion.h"
#include "MemoryManager.h"

VMProt VMProt::R = {
		.read = true,
		.write = false,
		.execute = false
};

VMProt VMProt::RW = {
		.read = true,
		.write = true,
		.execute = false
};

VMProt VMProt::RX = {
		.read = true,
		.write = false,
		.execute = true
};

VMProt VMProt::RWX = {
		.read = true,
		.write = true,
		.execute = true
};

VMRegion::VMRegion(kstd::Arc<VMObject> object, kstd::Arc<VMSpace> space, VirtualRange range, size_t object_start, VMProt prot):
	m_object(object),
	m_space(space),
	m_range(range),
	m_object_start(object_start),
	m_prot(prot)
{

}

VMRegion::~VMRegion() {
	m_space.with_locked([&](const kstd::Arc<VMSpace>& space) {
		auto unmap_res = space->unmap_region(*this);
		ASSERT(unmap_res.is_success());
	});
}

void VMRegion::set_prot(VMProt prot) {
	m_prot = prot;
}