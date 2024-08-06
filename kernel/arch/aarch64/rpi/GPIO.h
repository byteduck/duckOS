/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/kstd/types.h>

namespace RPi {
	class GPIO {
	public:
		enum Function: uint8_t {
			INPUT  = 0x0,
			OUTPUT = 0x1,
			ALT0   = 0x4,
			ALT1   = 0x5,
			ALT2   = 0x6,
			ALT3   = 0x7,
			ALT4   = 0x3,
			ALT5   = 0x2
		};

		enum PullMode: uint8_t {
			OFF =  0x0,
			DOWN = 0x1,
			UP =   0x2
		};

		static void set_pin_function(int pin, Function function);
		static void set_pin_pull(int pin, PullMode mode);

	private:
		static constexpr size_t base = 0x200000;
		enum Register: size_t {
			GPFSEL0 = base + 0x00,
			GPFSEL1 = base + 0x04,
			GPFSEL2 = base + 0x08,
			GPFSEL3 = base + 0x0C,
			GPFSEL4 = base + 0x10,
			GPFSEL5 = base + 0x14,

			GPSET0 = base + 0x1C,
			GPSET1 = base + 0x20,

			GPCLR0 = base + 0x28,
			GPCLR1 = base + 0x2C,

			GPPUD =     base + 0x94,
			GPPUDCLK0 = base + 0x98,
			CPPUDCLK1 = base + 0x9C
		};
	};
}
