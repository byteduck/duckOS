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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include "kstdio.h"
#include <kernel/kstd/kstddef.h>
#include <kernel/kstd/kstdlib.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/terminal/VirtualTTY.h>
#include <kernel/kstd/defines.h>
#include <kernel/IO.h>
#include <kernel/KernelMapper.h>
#include <kernel/interrupt/interrupt.h>
#include "cstring.h"
#include <kernel/filesystem/FileDescriptor.h>

kstd::shared_ptr<FileDescriptor> tty_desc(nullptr);
kstd::shared_ptr<VirtualTTY> tty(nullptr);
bool use_tty = false;
bool panicking = false;

void putch(char c){
	if(panicking)
		tty->get_terminal()->write_char(c);
	else if(tty && use_tty)
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

void print(const char* str){
	if(panicking)
		tty->get_terminal()->write_chars(str, strlen(str));
	else if(tty && use_tty)
		tty_desc->write((uint8_t*) str, strlen(str));
	while(*str)
		serial_putch(*(str++));
}

void printf(const char* fmt, ...) {
	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);
}

void vprintf(const char* fmt, va_list argp){
	const char *p;
	int i;
	char *s;
	char fmtbuf[256];

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
}

bool panicked = false;

[[noreturn]] void PANIC(const char* error, const char* msg, ...){
	asm volatile("cli");
	TaskManager::enabled() = false;

	panicking = true;
	tty->set_graphical(false);
	printf("\033[41;97m\033[2J"); //Red BG, bright white FG
	print("Whoops! Something terrible happened.\nIf you weren't expecting this, feel free to open an issue on GitHub to report it.\nHere are the details:\n");
	printf("%s\n", error);

	va_list list;
	va_start(list, msg);
	vprintf(msg, list);
	va_end(list);

	//Printing the stacktrace may panic if done early in kernel initialization. Don't print stacktrace in a nested panic.
#ifdef DEBUG
	if(!panicked) {
		panicked = true;
		printf("\n\nStack trace:\n");
		KernelMapper::print_stacktrace();
	}
#endif

	asm volatile("cli; hlt");
	while(1);
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