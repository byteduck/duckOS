/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "VMRegion.h"
#include "MemoryManager.h"

VMRegion::VMRegion(Ptr<VMObject> object, Ptr<VMSpace> space, size_t start, size_t size, VMProt prot):
	m_object(object),
	m_space(space),
	m_start(start),
	m_size(size),
	m_prot(prot)
{

}

VMRegion::~VMRegion() {
	m_space.with_locked([&](const Ptr<VMSpace>& space) {
		auto unmap_res = space->unmap_region(*this);
		ASSERT(unmap_res.is_success());
	});
}

void VMRegion::set_cow(bool cow) {
	m_prot.cow = cow;
}
