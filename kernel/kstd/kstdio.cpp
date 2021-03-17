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
#include <kernel/kstd/stdlib.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/terminal/VirtualTTY.h>
#include <kernel/kstd/defines.h>

kstd::shared_ptr<FileDescriptor> tty_desc(nullptr);
kstd::shared_ptr<VirtualTTY> tty(nullptr);

void putch(char c){
	if(tty) tty_desc->write((uint8_t*) &c, 1);
	serial_putch(c);
}

void serial_putch(char c) {
	static bool serial_inited = false;
	if(!serial_inited) {
		outb(0x3F9, 0x00);
		outb(0x3FB, 0x80);
		outb(0x3F8, 0x02);
		outb(0x3F9, 0x00);
		outb(0x3FB, 0x03);
		outb(0x3FA, 0xC7);
		outb(0x3FC, 0x0B);
		serial_inited = true;
	}

	while (!(inb(0x3FD) & 0x20u));

	if(c == '\n') {
		outb(0x3F8, '\r');
		while (!(inb(0x3FD) & 0x20u));
	}
	outb(0x3F8, c);
}

void print(char* str){
	if(tty) tty_desc->write((uint8_t*) str, strlen(str));
	while(*str) serial_putch(*(str++));
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
				toUpper(s);
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
	printf("\033[41;97m\033[2J"); //Red BG, bright white FG, clear screen
	print("Good job, you crashed it.\nHere are the details, since you probably need them.\nTry not to mess it up again.\n\n");
	printf("%s\n", error);
	printf("%s\n", msg);
	while(hang);
	TaskManager::enabled() = true;
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