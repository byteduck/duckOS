/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */
#include "KernelTest.h"
#include "../memory/PageDirectory.h"
#include "../memory/MemoryRegion.h"
#include "../random.h"

#define NUM_REGIONS 100

struct RegionProperties {
	size_t location;
	size_t size;
};

KERNEL_TEST(allocate_and_free_regions) {
	PageDirectory dir;
	LinkedMemoryRegion regions[NUM_REGIONS];
	RegionProperties virt_regions[NUM_REGIONS];
	RegionProperties phys_regions[NUM_REGIONS];

	auto test_region = [&](LinkedMemoryRegion region) {
		ENSURE(region.phys && region.virt);
		ENSURE(region.phys->size == region.virt->size);
		ENSURE(region.phys->used && region.virt->used);
		ENSURE(!region.phys->is_shm && !region.phys->is_shm);
	};

	// Allocate a bunch of randomly-sized regions
	for(int i = 0; i < NUM_REGIONS; i++) {
		auto& region = regions[i];
		region = dir.allocate_region(rand() % (PAGE_SIZE * 5), true);
		virt_regions[i] = {region.virt->start, region.virt->size};
		phys_regions[i] = {region.phys->start, region.phys->size};
		test_region(region);
	}

	// Test them all again to make sure they're correct
	for(int i = 0; i < NUM_REGIONS; i++) {
		auto& region = regions[i];
		test_region(region);
		ENSURE_EQ(region.virt, dir.vmem_map().find_region(virt_regions[i].location));
		ENSURE_EQ(region.phys, MemoryManager::inst().pmem_map().find_region(phys_regions[i].location));
		ENSURE_EQ(region.virt->start, virt_regions[i].location);
		ENSURE_EQ(region.virt->size, virt_regions[i].size);
		ENSURE_EQ(region.phys->start, phys_regions[i].location);
		ENSURE_EQ(region.phys->size, phys_regions[i].size);
	}

	// Free them
	for(int i = 0; i < NUM_REGIONS; i++)
		dir.free_region(regions[i]);
}