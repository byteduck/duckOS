/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/kstd/types.h>

namespace RPi {
	class Mailbox {
	public:
		enum Channel: uint8_t {
			POWER = 0,
			FB    = 1,
			VUART = 2,
			VCHIQ = 3,
			LEDS  = 4,
			BTNS  = 5,
			TOUCH = 6,
			COUNT = 7,
			PROP  = 8
		};

		enum CommandTag: uint8_t {
			REQUEST = 0
		};

		static bool call(void* buffer, Channel channel);

	private:
		static constexpr size_t phys_base = 0x3F00B880;
		enum Register: size_t {
			READ =   0x0,
			POLL =   0x10,
			SENDER = 0x14,
			STATUS = 0x18,
			CONFIG = 0x1C,
			WRITE =  0x20
		};

		enum Status: uint32_t {
			FULL = 0x80000000,
			EMPTY = 0x40000000,
			RESPONSE = 0x80000000
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