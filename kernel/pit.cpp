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

    Copyright (c) Byteduck 2016-$YEAR. All rights reserved.
*/

#include <kernel/kstddef.h>
#include <kernel/pit.h>
#include <kernel/kstdio.h>
#include <kernel/tasking/TaskManager.h>

extern bool tasking_enabled;

void pit_handler(){
	asm volatile("pusha");
	outb(0x20, 0x20);
	asm volatile("popa");
	//asm volatile("iret");
}

static inline void pit_send_data(uint16_t data, uint8_t counter){
	uint8_t port = (counter==0) ? PIT_COUNTER0 : ((counter==1) ? PIT_COUNTER1 : PIT_COUNTER2);
	outb(port, (uint8_t)data);
}

void pit_init(uint32_t frequency){
	uint16_t divisor = (uint16_t)( 1193181 / (uint16_t)frequency);

	uint8_t ocw = 0;
	ocw = (ocw & ~0xE) | 0x6;
	ocw = (ocw & ~0x30) | 0x30;
	ocw = (ocw & ~0xC0) | 0;
	outb(PIT_CMD, ocw);

	// set frequency rate
	pit_send_data(divisor & 0xff, 0);
	pit_send_data((divisor >> 8) & 0xff, 0);
}
