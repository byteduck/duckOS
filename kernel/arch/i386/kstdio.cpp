/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include <kernel/IO.h>

void serial_putch(char c) {
	static bool serial_inited = false;
	if(!serial_inited) {
		IO::outb(0x3F9, 0x00);
		IO::outb(0x3FB, 0x80);
		IO::outb(0x3F8, 0x02);
		IO::outb(0x3F9, 0x00);
		IO::outb(0x3FB, 0x03);
		IO::outb(0x3FA, 0xC7);
		IO::outb(0x3FC, 0x0B);
		serial_inited = true;
	}

	while (!(IO::inb(0x3FD) & 0x20u));

	if(c == '\n') {
		IO::outb(0x3F8, '\r');
		while (!(IO::inb(0x3FD) & 0x20u));
	}
	IO::outb(0x3F8, c);
}