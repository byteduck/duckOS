/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "VMRegion.h"
#include "MemoryManager.h"

VMRegion::VMRegion(kstd::shared_ptr<VMObject> object, size_t start, size_t size):
	m_object(object),
	m_start(start),
	m_size(size)
{

}

VMRegion::~VMRegion() {
	if(is_kernel()) {
		auto unmap_res = MM.kernel_space().unmap_region(*this);
		ASSERT(unmap_res.is_success());
	}
}
