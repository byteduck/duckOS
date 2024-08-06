/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "kernel/device/I8042.h"
#include "AC97Device.h"
#include "BochsVGADevice.h"
#include "kernel/device/MultibootVGADevice.h"

extern multiboot_info mboot_header;

void Device::arch_init() {
	I8042::init();
	auto dev = AC97Device::detect();
	if (!dev.is_error())
		dev.value().leak_ref(); // Yes..

	BochsVGADevice* bochs_vga = BochsVGADevice::create();
	if(!bochs_vga) {
		//We didn't find a bochs VGA device, try using the multiboot VGA device
		auto* mboot_vga = MultibootVGADevice::create(&mboot_header);
		if(!mboot_vga || mboot_vga->is_textmode())
			PANIC("MBOOT_TEXTMODE", "duckOS doesn't support textmode.");
	}
}