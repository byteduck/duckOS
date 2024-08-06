/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "GPIO.h"
#include "../aarch64util.h"
#include "MMIO.h"

using namespace RPi;

// BCM2835 manual section 6.1
void GPIO::set_pin_function(int pin, Function function) {
	if (pin < 0 || pin > 53)
		return; // Invalid pin

	auto reg = static_cast<Register>(Register::GPFSEL0 + (size_t) (pin / 10) * 4);
	auto lobit = (pin % 10) * 3;

	auto selector = MMIO::peek<uint32_t>(reg);
	selector &= ~(7 << lobit);
	selector |= function << lobit;
	MMIO::poke<uint32_t>(reg, selector);
}

void GPIO::set_pin_pull(int pin, GPIO::PullMode mode) {
	if (pin < 0 || pin > 53)
		return; // Invalid pin

	auto clk_reg = static_cast<Register>(Register::GPPUDCLK0 + (size_t) (pin / 32) * 4);

	MMIO::poke<uint32_t>(GPPUD, mode);
	Aarch64::bne_delay(150);
	MMIO::poke<uint32_t>(clk_reg, 1 << (pin % 32));
	Aarch64::bne_delay(150);
	MMIO::poke<uint32_t>(GPPUD, 0);
	MMIO::poke<uint32_t>(clk_reg, 0);
}
