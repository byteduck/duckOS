/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "MiniUART.h"
#include "GPIO.h"
#include "MMIO.h"

using namespace RPi;


void MiniUART::init() {
	// Setup pins 14 and 15 for UART by setting their function and turning off pull down/up
	GPIO::set_pin_function(14, GPIO::ALT5); // TXD1
	GPIO::set_pin_function(15, GPIO::ALT5); // RXD1
	GPIO::set_pin_pull(14, GPIO::OFF);
	GPIO::set_pin_pull(15, GPIO::OFF);

	// Enable mini UART
	MMIO::poke<uint32_t>(AUX_ENABLES, 1); // Enable
	MMIO::poke<uint32_t>(AUX_MU_CNTL, 0); // Disable flow control, rx, and tx
	MMIO::poke<uint32_t>(AUX_MU_IER, 0); // Disable interrupts
	MMIO::poke<uint32_t>(AUX_MU_LCR, 3); // 8-bit mode
	MMIO::poke<uint32_t>(AUX_MU_IIR, 0xc6);
	MMIO::poke<uint32_t>(AUX_MU_MCR, 0); // RTS high
	MMIO::poke<uint32_t>(AUX_MU_BAUD, 270); // 115200 baud
	MMIO::poke<uint32_t>(AUX_MU_CNTL, 3); // Enable rx and tx
}

void MiniUART::tx(char c) {
	while (!(MMIO::peek<uint32_t>(AUX_MU_LSR) & 0x20));
	MMIO::poke<uint32_t>(AUX_MU_IO, c);
}

void MiniUART::puts(const char* str) {
	while (*str)
		tx(*(str++));
}

char MiniUART::rx() {
	while (!(MMIO::peek<uint32_t>(AUX_MU_LSR) & 0x01));
	return MMIO::peek<uint32_t>(AUX_MU_IO) & 0xFF;
}
