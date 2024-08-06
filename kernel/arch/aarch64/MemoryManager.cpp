/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include <kernel/memory/MemoryManager.h>
#include "rpi/DeviceInfo.h"

void MemoryManager::setup_device_memory_map() {
	size_t kernel_start_page = (KERNEL_START - HIGHER_HALF) / PAGE_SIZE;
	size_t kernel_end_page = (KERNEL_END - HIGHER_HALF + PAGE_SIZE - 1) / PAGE_SIZE;
	size_t sdram_start_page = RPi::DeviceInfo::inst().sdram_start() / PAGE_SIZE;
	if (sdram_start_page == 0)
		sdram_start_page = 1; // Can't start sdram at page zero or else we encounter issues
	size_t sdram_end_page = (RPi::DeviceInfo::inst().sdram_size() + RPi::DeviceInfo::inst().sdram_start()) / PAGE_SIZE;

	// SDRAM before kernel
	if (kernel_start_page - sdram_start_page) {
		m_physical_regions.push_back(new PhysicalRegion(
				sdram_start_page,
				kernel_start_page - sdram_start_page,
				false, false));
	}

	// Kernel (reserved)
	m_physical_regions.push_back(new PhysicalRegion(
			kernel_start_page,
			kernel_end_page - kernel_start_page,
			true, false));

	// SDRAM after kernel
	m_physical_regions.push_back(new PhysicalRegion(
			kernel_end_page,
			sdram_end_page - kernel_end_page,
			false, false));

	// MMIO (reserved)
	m_physical_regions.push_back(new PhysicalRegion(
			RPi::DeviceInfo::inst().mmio_start() / PAGE_SIZE,
			RPi::DeviceInfo::inst().mmio_size() / PAGE_SIZE,
			true, false));

	usable_bytes_ram = RPi::DeviceInfo::inst().sdram_size();
	total_bytes_ram = usable_bytes_ram;
	mem_lower_limit = RPi::DeviceInfo::inst().sdram_start();
	mem_upper_limit = RPi::DeviceInfo::inst().mmio_start() + RPi::DeviceInfo::inst().mmio_size();
}