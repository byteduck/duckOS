/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/kstd/types.h>
#include <kernel/memory/VMRegion.h>
#include "MMIO.h"

namespace RPi {
	class MiniUART {
	public:
		static void init();
		static void tx(char c);
		static void puts(const char* str);
		static char rx();

	private:
		static constexpr size_t base = 0x215000;
		enum Register: size_t {
			AUX_IRQ =        base + 0x00,
			AUX_ENABLES =    base + 0x04,
			AUX_MU_IO =      base + 0x40,
			AUX_MU_IER =     base + 0x44,
			AUX_MU_IIR =     base + 0x48,
			AUX_MU_LCR =     base + 0x4C,
			AUX_MU_MCR =     base + 0x50,
			AUX_MU_LSR =     base + 0x54,
			AUX_MU_MSR =     base + 0x58,
			AUX_MU_SCRATCH = base + 0x5C,
			AUX_MU_CNTL =    base + 0x60,
			AUX_MU_STAT =    base + 0x64,
			AUX_MU_BAUD =    base + 0x68
		};
	};
}