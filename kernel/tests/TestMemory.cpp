/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */
#include "KernelTest.h"
#include <kernel/memory/PageDirectory.h>
#include "../random.h"

#define NUM_REGIONS 100

KERNEL_TEST(allocate_and_free_regions) {
	kstd::Arc<VMRegion> regions[NUM_REGIONS];

	// Allocate a bunch of randomly-sized regions
	for(int i = 0; i < NUM_REGIONS; i++) {
		auto& region = regions[i];
		region = MM.alloc_kernel_region(rand() % (PAGE_SIZE * 5));
		regions[i] = region;
		memset((void*) region->start(), 0, region->size());
	}

	for(int i = 0; i < NUM_REGIONS; i++) {
		auto start = regions[i]->start();
		ENSURE(MM.kernel_page_directory.is_mapped(start, true));
		regions[i].reset();
		ENSURE(!MM.kernel_page_directory.is_mapped(start, true));
	}
}