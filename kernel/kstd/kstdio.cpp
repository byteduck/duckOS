/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <kernel/kstd/kstddef.h>
#include <kernel/kstd/kstdio.h>
#include <kernel/kstd/kstdlib.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/terminal/VirtualTTY.h>
#include <kernel/kstd/defines.h>
#include <kernel/IO.h>
#include <kernel/KernelMapper.h>
#include <kernel/interrupt/interrupt.h>
#include "cstring.h"

kstd::shared_ptr<FileDescriptor> tty_desc(nullptr);
kstd::shared_ptr<VirtualTTY> tty(nullptr);
bool use_tty = false;

void putch(char c){
	if(tty && use_tty)
		tty_desc->write((uint8_t*) &c, 1);
	serial_putch(c);
}

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

void print(char* str){
	if(tty && use_tty)
		tty_desc->write((uint8_t*) str, strlen(str));
	while(*str)
		serial_putch(*(str++));
}

void printf(const char* fmt, ...){
	const char *p;
	va_list argp;
	int i;
	char *s;
	char fmtbuf[256];

	va_start(argp, fmt);

	for(p = fmt; *p != '\0'; p++){
		if(*p != '%'){
			putch(*p);
			continue;
		}
		switch(*++p){
			case 'c':
				i = va_arg(argp, int);
				putch(i);
				break;

			case 'd':
				i = va_arg(argp, int);
				s = itoa(i, fmtbuf, 10);
				print(s);
				break;

			case 's':
				s = va_arg(argp, char *);
				print(s);
				break;

			case 'x':
				i = va_arg(argp, int);
				s = itoa(i, fmtbuf, 16);
				print(s);
				break;

			case 'X':
				i = va_arg(argp, int);
				s = itoa(i, fmtbuf, 16);
				to_upper(s);
				print(s);
				break;

			case 'b':
				i = va_arg(argp, int);
				s = itoa(i, fmtbuf, 2);
				print(s);
				break;

			case '%':
				putch('%');
				break;
		}
	}
	va_end(argp);
}

void PANIC(char *error, char *msg, bool hang){
	TaskManager::enabled() = false;
	use_tty = true;
	printf("\n\033[41;97m"); //Red BG, bright white FG
	print("Good job, you crashed it.\nHere are the details, since you probably need them.\nTry not to mess it up again.\n\n");
	printf("%s\n", error);
	printf("%s\n", msg);

	printf("Stack trace:\n");
	KernelMapper::print_stacktrace();

	if(!hang) {
		TaskManager::enabled() = true;
	} else {
		Interrupt::Disabler disabler;
		Interrupt::NMIDisabler disabler2;
		while(true);
	}
}

void clearScreen(){
	if(!tty) return;
	tty->clear();
}

void setup_tty() {
	tty = VirtualTTY::current_tty();
	tty_desc = kstd::make_shared<FileDescriptor>(tty);
	tty_desc->set_options(O_WRONLY);
}