/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Futex.h"
#include "../memory/MemoryManager.h"

Futex::Futex(kstd::Arc<VMObject> object, size_t offset_in_object):
	m_object(kstd::move(object)),
	m_k_region(MM.map_object(m_object)),
	m_var((Atomic<int>*) (m_k_region->start() + offset_in_object))
{
	ASSERT(offset_in_object + sizeof(*m_var) <= m_object->size());
}

bool Futex::is_ready() {
	return m_var->load(MemoryOrder::Relaxed) > 0;
}

bool Futex::can_read(const FileDescriptor& fd) {
	return is_ready();
}
