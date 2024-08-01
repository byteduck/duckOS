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
#include <kernel/memory/Memory.h>
#include <kernel/constructors.h>

// TODO: We could unmap this once it's no longer needed.
__attribute__((aligned(8))) char __early_stack[0x4000];

[[noreturn]] void aarch64_late_init();

extern "C" int kmain(size_t mbootptr);

extern "C" [[noreturn]] void aarch64init() {
	// We're currently mapped at a low physical address, so no touching any variables yet.
	setup_exception_level();

	// Init MMU
	Aarch64::MMU::mmu_init();

	call_global_constructors();

	// Jump to high memory and correct sp
	asm volatile (
			"ldr x0, =1f                  \n" // Continue execution in high mem
			"br x0                        \n"
			"1:                           \n"

			"mov x0, sp                   \n" // OR high bits into sp
			"mov x1, #" STR(HIGHER_HALF) "\n"
			"orr x0, x0, x1               \n"
			"mov sp, x0                   \n"

			"b kmain                      \n" // Jump to kinit
			::: "x0", "x1");

	ASSERT(false);
}

extern "C" [[noreturn]] void unknown_el() {
	// We may end up here if we boot in an unknown EL
//	RPi::MiniUART::puts("Booted in el");
//	RPi::MiniUART::tx('0' + get_el());
//	RPi::MiniUART::puts(", cannot handle. Halting.\n");
	Processor::halt();
}