/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "KBuffer.h"
#include "MemoryManager.h"

KBuffer::KBuffer(const kstd::Arc<VMRegion>& region):
	m_region(region), WriteableBytes((uint8_t*) region->start(), region->size()) {

}

ResultRet<kstd::Arc<KBuffer>> KBuffer::alloc(size_t size) {
	return kstd::Arc(new KBuffer {MM.alloc_kernel_region(size)});
}


