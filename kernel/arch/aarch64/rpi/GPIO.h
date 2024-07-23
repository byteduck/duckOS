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
		static constexpr size_t phys_base = 0x3F200000;
		enum Register: size_t {
			GPFSEL0 = 0x00,
			GPFSEL1 = 0x04,
			GPFSEL2 = 0x08,
			GPFSEL3 = 0x0C,
			GPFSEL4 = 0x10,
			GPFSEL5 = 0x14,

			GPSET0 = 0x1C,
			GPSET1 = 0x20,

			GPCLR0 = 0x28,
			GPCLR1 = 0x2C,

			GPPUD =     0x94,
			GPPUDCLK0 = 0x98,
			CPPUDCLK1 = 0x9C
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
