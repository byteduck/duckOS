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
#include <kernel/interrupt/idt.h>
#include <common/cstring.h>

struct IDTEntry idt[256];
struct IDTPointer idtp;

void idt_set_gate(uint8_t num, uint32_t loc, uint16_t selector, uint8_t attrs){
	idt[num].offset_low = (loc & 0xFFFF);
	idt[num].offset_high = (loc >> 16) & 0xFFFF;
	idt[num].selector = selector;
	idt[num].zero = 0;
	idt[num].attrs = attrs;
}

void register_idt(){
	idtp.size = (sizeof(struct IDTEntry) * 256) - 1;
	idtp.offset = (int)&idt;
	
	memset(&idt, 0, sizeof(struct IDTEntry) * 256);
	
	idt_load();
}