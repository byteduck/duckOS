/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "kernel/device/I8042.h"
#include "AC97Device.h"

void Device::arch_init() {
	I8042::init();
	auto dev = AC97Device::detect();
	if (!dev.is_error())
		dev.value().leak_ref(); // Yes..
}