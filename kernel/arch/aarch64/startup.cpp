/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "aarch64util.h"

#include "rpi/MiniUART.h"
#include "rpi/Mailbox.h"
#include "rpi/Framebuffer.h"
#include "Processor.h"
#include <kernel/kstd/kstdio.h>
#include "asm/exception.h"
#include "MMU.h"

extern "C" [[noreturn]] void aarch64init() {
	// We're currently mapped at a low physical address, so no touching any variables yet.
	setup_exception_level();
	// Init MMU
	Aarch64::MMU::mmu_init();

	while(1); // TODO

	// Setup MiniUART for output
	RPi::MiniUART::init();
	RPi::MiniUART::puts("Booting, in el");
	RPi::MiniUART::tx('0' + get_el());
	RPi::MiniUART::tx('\n');

	RPi::Framebuffer::init();
	RPi::MiniUART::puts("Framebuffer inited!\n");

	while (1);
}

extern "C" [[noreturn]] void unknown_el() {
	// We may end up here if we boot in an unknown EL
//	RPi::MiniUART::puts("Booted in el");
//	RPi::MiniUART::tx('0' + get_el());
//	RPi::MiniUART::puts(", cannot handle. Halting.\n");
	Processor::halt();
}