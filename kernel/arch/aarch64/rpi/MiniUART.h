/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/kstd/types.h>

namespace RPi {
	class MiniUART {
	public:
		static void init();
		static void tx(char c);
		static void puts(const char* str);
		static char rx();

	private:
		static constexpr size_t phys_base = 0x3F215000;
		enum Register: size_t {
			AUX_IRQ =        0x00,
			AUX_ENABLES =    0x04,
			AUX_MU_IO =      0x40,
			AUX_MU_IER =     0x44,
			AUX_MU_IIR =     0x48,
			AUX_MU_LCR =     0x4C,
			AUX_MU_MCR =     0x50,
			AUX_MU_LSR =     0x54,
			AUX_MU_MSR =     0x58,
			AUX_MU_SCRATCH = 0x5C,
			AUX_MU_CNTL =    0x60,
			AUX_MU_STAT =    0x64,
			AUX_MU_BAUD =    0x68
		};

		template<typename T>
		inline static T get(Register reg) {
			return *((T*) (phys_base + reg));
		}

		template<typename T>
		inline static void set(Register reg, T val) {
			*((T*) (phys_base + reg)) = val;
		}
	};
}