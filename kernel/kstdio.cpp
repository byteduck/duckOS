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

#include <kernel/kstddef.h>
#include <kernel/kstdio.h>
#include <kernel/memory/Memory.h>
#include <common/stdlib.h>
#include <kernel/font8x8/font8x8_latin.h>
#include <kernel/device/BochsVGADevice.h>
#include <kernel/tasking/TaskManager.h>

int xpos = 0;
int ypos = 0;
uint32_t num_columns = 80;
uint32_t num_rows = 25;
char ccolor = 0x0f;
uint8_t* vidmem = nullptr;
bool graphical_mode = false;

//Every single print statement ultimately uses this method.
void putch_color(char c, char color){
	serial_putch(c);
	if(vidmem == nullptr) return;
	if(c == '\r'){
		xpos = 0;
	}else if(c == '\n'){
		xpos = 0;
		ypos++;
	}else if(c == '\t'){
		for(uint8_t i = 0; i < 5; i++)
			putch_color(' ',color);
	} else {
		if(graphical_mode) graphical_putch_color(c, color);
		else textmode_putch_color(c, color);
		xpos++;
		if(xpos >= num_columns){
			ypos++;
			xpos = 0;
		}
		if(!graphical_mode) update_cursor();
	}
}

void graphical_putch_color(char c, char color) {
	uint32_t* framebuffer = (uint32_t*) vidmem;

	while(ypos >= num_rows) {
		//TODO: Better way of scrolling other than memcpying framebuffer
		memcpy(framebuffer, framebuffer + 8 * (num_columns * 8), (num_rows - 1) * 8 * num_columns * 8 * 4);
		memset(framebuffer + (8 * (num_rows - 1)) * (num_columns * 8), 0, num_columns * 8 * 8 * 4);
		ypos--;
	}

	uint16_t pixel_xpos = xpos * 8;
	uint16_t pixel_ypos = ypos * 8;
	for(uint16_t x = 0; x < 8; x++) {
		for(uint16_t y = 0; y < 8; y++) {
			framebuffer[(x + pixel_xpos) + (y + pixel_ypos) * num_columns * 8] = ((font8x8_basic[(int)c][y] >> x) & 0x1u) ? 0xFFFFFFu : 0x0u;
		}
	}
}

void textmode_putch_color(char c, char color) {
	while(ypos >= num_rows) {
		uint16_t i = 80*2;
		while(i < 80*25*2){
			vidmem[i-(80*2)] = vidmem[i];
			i++;
		}
		i = 80*2*24;
		while(i < 80*25*2){
			vidmem[i++] = ' ';
			vidmem[i++] = 0x07;
		}
		ypos--;
	}

	int pos = (xpos+(ypos*80))*2;
	vidmem[pos] = c;
	vidmem[pos+1] = color;
}

void putch(char c){
	if(c == '\b')
		backspace();
	else if(c)
		putch_color(c, ccolor);
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

	if(c == '\n') outb(0x3F8, '\r');
	outb(0x3F8, c);
}




void print_color(char* c, char color){
	int i = 0;
	while(c[i] != 0){
		putch_color(c[i], color);
		i++;
	}
}

void print(char* c){
	print_color(c, ccolor);
}

void println_color(char* c, char color){
	print_color(c, color);
	print_color("\n", color);
}

void println(char* c){
	println_color(c,ccolor);
}

void setColor(char color){
	ccolor = color;
}

void printf(char *fmt, ...){
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

void backspace(){
	serial_putch('\b');
	if(xpos != 0){
		xpos--;
		putch(' ');
		xpos--;
		update_cursor();
	}
}

void PANIC(char *error, char *msg, bool hang){
	clearScreen();
	setAllColor(0x9f);
	setAllColor(0x9f);
	println("Good job, you crashed it.\nAnyway, here's the details, since you probably need them.\nDon't mess it up again.\n");
	println(error);
	println(msg);
	if(hang) cli();
	while(hang);
}

void clearScreen(){
	if(vidmem == nullptr) return;
	if(!graphical_mode) {
		for (int y = 0; y < 25; y++) {
			for (int x = 0; x < 80; x++) {
				vidmem[(x + (y * 80)) * 2] = ' ';
			}
		}
	} else {
		memset(vidmem, 0, num_columns * num_rows * 8 * 8 * 4);
	}
	xpos = 0;
	ypos = 0;
}

void setAllColor(char color){
	for(int y=0; y<25; y++){
		for(int x=0; x<80; x++){
			vidmem[(x+(y*80))*2+1] = color;
		}
	}
	setColor(color);
}

void update_cursor(){
	uint16_t position=(ypos*80) + xpos;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(position&0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((position>>8)&0xFF));
}

void set_graphical_mode(uint32_t width, uint32_t height, void* framebuffer) {
	graphical_mode = true;
	vidmem = (uint8_t*) framebuffer;
	num_columns = width / 8;
	num_rows = height / 8;
	xpos = 0;
	ypos = 0;
}

void set_text_mode(uint32_t width, uint32_t height, void* framebuffer) {
	vidmem = (uint8_t*) framebuffer;
	graphical_mode = false;
	num_columns = width;
	num_rows = height;
	xpos = 0;
	ypos = 0;
}