/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include <kernel/device/Device.h>
#include "rpi/MiniUART.h"
#include "rpi/Framebuffer.h"

void Device::arch_init() {
	RPi::MMIO::inst();
	RPi::Framebuffer::init();
}