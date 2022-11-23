/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "PhysicalPage.h"
#include "../kstd/kstdio.h"
#include "MemoryManager.h"

PageIndex PhysicalPage::index() const {
	return this - &MemoryManager::inst().get_physical_page(0);
}

void PhysicalPage::release() {
	ASSERT(allocated.ref_count.load(MemoryOrder::Relaxed) == 0);
	if(!allocated.reserved)
		MemoryManager::inst().free_physical_page(index());
}