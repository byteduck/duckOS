/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/kstd/types.h>

namespace RPi {
	class DeviceInfo {
	public:
		static DeviceInfo& inst();

		enum Model {
			RPi3b
		};

		size_t sdram_start() const;
		size_t sdram_size() const;
		size_t mmio_start() const;
		size_t mmio_size() const;

		Model model() const;

	private:
		DeviceInfo();
		static DeviceInfo* s_inst;
	};
}
