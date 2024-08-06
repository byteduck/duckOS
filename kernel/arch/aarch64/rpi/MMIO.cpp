/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "MMIO.h"
#include <kernel/memory/MemoryManager.h>

using namespace RPi;

MMIO* MMIO::s_inst = nullptr;

MMIO& MMIO::inst() {
	if (!MM.is_paging_setup())
		PANIC("MMIO_BEFORE_PAGING", "MMIO accessed before paging setup.");
	if (__builtin_expect(!s_inst, false))
		s_inst = new MMIO();
	return *s_inst;
}

MMIO::MMIO() {
	m_region = MM.map_device_region(phys_base, phys_size);
}
