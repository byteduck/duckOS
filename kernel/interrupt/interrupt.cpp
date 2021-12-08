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

#include "interrupt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"

extern "C" void asm_syscall_handler();
extern "C" void preempt_now_asm();

void Interrupt::init() {
	//Register the IDT
	Interrupt::register_idt();
	//Setup ISR handlers
	Interrupt::isr_init();
	//Setup the syscall handler
	Interrupt::idt_set_gate(0x80, (unsigned)asm_syscall_handler, 0x08, 0xEF);
	//Setup the immediate preemption handler
	Interrupt::idt_set_gate(0x81, (unsigned)preempt_now_asm, 0x08, 0x8E);
	//Setup IRQ handlers
	Interrupt::irq_init();
	//Start interrupts
	asm volatile("sti");
}