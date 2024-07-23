/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "MiniUART.h"
#include "GPIO.h"

using namespace RPi;

void MiniUART::init() {
	// Setup pins 14 and 15 for UART by setting their function and turning off pull down/up
	GPIO::set_pin_function(14, GPIO::ALT5); // TXD1
	GPIO::set_pin_function(15, GPIO::ALT5); // RXD1
	GPIO::set_pin_pull(14, GPIO::OFF);
	GPIO::set_pin_pull(15, GPIO::OFF);

	// Enable mini UART
	set<uint32_t>(AUX_ENABLES, 1); // Enable
	set<uint32_t>(AUX_MU_CNTL, 0); // Disable flow control, rx, and tx
	set<uint32_t>(AUX_MU_IER, 0); // Disable interrupts
	set<uint32_t>(AUX_MU_LCR, 3); // 8-bit mode
	set<uint32_t>(AUX_MU_IIR, 0xc6);
	set<uint32_t>(AUX_MU_MCR, 0); // RTS high
	set<uint32_t>(AUX_MU_BAUD, 270); // 115200 baud
	set<uint32_t>(AUX_MU_CNTL, 3); // Enable rx and tx
}

void MiniUART::tx(char c) {
	while (!(get<uint32_t>(AUX_MU_LSR) & 0x20));
	set<uint32_t>(AUX_MU_IO, c);
}

void MiniUART::puts(const char* str) {
	while (*str)
		tx(*(str++));
}

char MiniUART::rx() {
	while (!(get<uint32_t>(AUX_MU_LSR) & 0x01));
	return get<uint32_t>(AUX_MU_IO) & 0xFF;
}
