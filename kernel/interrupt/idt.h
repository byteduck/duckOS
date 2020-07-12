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

#ifndef IDT_H
#define IDT_H

namespace Interrupt {
	struct IDTEntry {
		unsigned short offset_low;  //Offset bits 0-15
		unsigned short selector;    //A code segment selector in the GDT
		unsigned char zero;         //Always 0
		unsigned char attrs;        //Type & Attributes
		unsigned short offset_high; //Offset bits 16-31
	}__attribute__((packed));

	struct IDTPointer {
		unsigned short size;
		unsigned int offset;
	}__attribute__((packed));

	extern "C" void idt_load();

	void idt_set_gate(uint8_t num, uint32_t loc, uint16_t selector, uint8_t flags);

	void register_idt();
}

#endif