/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include <kernel/memory/MemoryManager.h>
#include "rpi/DeviceInfo.h"

void MemoryManager::setup_device_memory_map() {
	size_t kernel_start_page = (KERNEL_START - HIGHER_HALF) / PAGE_SIZE;
	size_t kernel_end_page = (KERNEL_END - HIGHER_HALF + PAGE_SIZE - 1) / PAGE_SIZE;
	size_t sdram_end_page = (RPi::DeviceInfo::inst().sdram_size() + RPi::DeviceInfo::inst().sdram_start()) / PAGE_SIZE;
	m_physical_regions.push_back(new PhysicalRegion(
			RPi::DeviceInfo::inst().sdram_start() / PAGE_SIZE,
			kernel_start_page - (RPi::DeviceInfo::inst().sdram_start() / PAGE_SIZE),
			false, false));
	m_physical_regions.push_back(new PhysicalRegion(
			kernel_start_page,
			kernel_end_page - kernel_start_page,
			false, true));
	m_physical_regions.push_back(new PhysicalRegion(
			kernel_end_page,
			sdram_end_page - kernel_end_page,
			false, false));
	m_physical_regions.push_back(new PhysicalRegion(
			RPi::DeviceInfo::inst().mmio_start() / PAGE_SIZE,
			RPi::DeviceInfo::inst().mmio_size() / PAGE_SIZE,
			true, true));
	usable_bytes_ram = RPi::DeviceInfo::inst().sdram_size();
	total_bytes_ram = usable_bytes_ram;
	mem_lower_limit = RPi::DeviceInfo::inst().sdram_start();
	mem_upper_limit = RPi::DeviceInfo::inst().mmio_start() + RPi::DeviceInfo::inst().mmio_size();
}