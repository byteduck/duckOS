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

#include "isr.h"
#include "interrupt.h"
#include <kernel/kstd/kstddef.h>
#include <kernel/memory/MemoryManager.h>
#include <kernel/kstd/kstdio.h>
#include <kernel/interrupt/idt.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/tasking/Signal.h>
#include <kernel/tasking/Thread.h>
#include <kernel/tasking/Process.h>

namespace Interrupt {
	void isr_init(){
		idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
		idt_set_gate(1, (unsigned)isr1, 0x08, 0x8E);
		idt_set_gate(2, (unsigned)isr2, 0x08, 0x8E);
		idt_set_gate(3, (unsigned)isr3, 0x08, 0x8E);
		idt_set_gate(4, (unsigned)isr4, 0x08, 0x8E);
		idt_set_gate(5, (unsigned)isr5, 0x08, 0x8E);
		idt_set_gate(6, (unsigned)isr6, 0x08, 0x8E);
		idt_set_gate(7, (unsigned)isr7, 0x08, 0x8E);
		idt_set_gate(8, (unsigned)isr8, 0x08, 0x8E);
		idt_set_gate(9, (unsigned)isr9, 0x08, 0x8E);
		idt_set_gate(10, (unsigned)isr10, 0x08, 0x8E);
		idt_set_gate(11, (unsigned)isr11, 0x08, 0x8E);
		idt_set_gate(12, (unsigned)isr12, 0x08, 0x8E);
		idt_set_gate(13, (unsigned)isr13, 0x08, 0x8E);
		idt_set_gate(14, (unsigned)isr14, 0x08, 0x8E);
		idt_set_gate(15, (unsigned)isr15, 0x08, 0x8E);
		idt_set_gate(16, (unsigned)isr16, 0x08, 0x8E);
		idt_set_gate(17, (unsigned)isr17, 0x08, 0x8E);
		idt_set_gate(18, (unsigned)isr18, 0x08, 0x8E);
		idt_set_gate(19, (unsigned)isr19, 0x08, 0x8E);
		idt_set_gate(20, (unsigned)isr20, 0x08, 0x8E);
		idt_set_gate(21, (unsigned)isr21, 0x08, 0x8E);
		idt_set_gate(22, (unsigned)isr22, 0x08, 0x8E);
		idt_set_gate(23, (unsigned)isr23, 0x08, 0x8E);
		idt_set_gate(24, (unsigned)isr24, 0x08, 0x8E);
		idt_set_gate(25, (unsigned)isr25, 0x08, 0x8E);
		idt_set_gate(26, (unsigned)isr26, 0x08, 0x8E);
		idt_set_gate(27, (unsigned)isr27, 0x08, 0x8E);
		idt_set_gate(28, (unsigned)isr28, 0x08, 0x8E);
		idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);
		idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);
		idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);
	}

	void handle_fault(const char* err, const char* panic_msg, uint32_t sig){
		if(!TaskManager::enabled() || TaskManager::current_thread()->is_kernel_mode() || TaskManager::is_preempting()){
			PANIC(err, panic_msg);
		} else {
			TaskManager::notify_current(sig);
		}
	}

	void fault_handler(struct Registers *r){
		if(r->num < 32){
			switch(r->num){
				case 0:
					handle_fault("DIVIDE_BY_ZERO", "Please don't do that.", SIGILL);
					break;

				case 13: //GPF
					handle_fault("GENERAL_PROTECTION_FAULT", "How did you manage to do that?", SIGILL);
					break;

				case 14: //Page fault
					if(!TaskManager::current_thread() || TaskManager::current_thread()->is_kernel_mode() || TaskManager::is_preempting()) {
						MemoryManager::inst().page_fault_handler(r);
					} else {
						size_t err_pos;
						asm volatile ("mov %%cr2, %0" : "=r" (err_pos));
						PageFault::Type type;
						if(r->err_code == FAULT_USER_READ)
							type = PageFault::Type::Read;
						else if(r->err_code == FAULT_USER_WRITE)
							type = PageFault::Type::Write;
						else
							type = PageFault::Type::Unknown;
						TaskManager::current_thread()->handle_pagefault({
							err_pos,
							r->eip,
						});
					}
					break;

				default:
					handle_fault("UNKNOWN_FAULT", "What did you do?", SIGILL);
			}
		}
	}
}

void print_regs(struct Registers *r){
	asm volatile("mov %%ss, %%eax":"=a"(r->ss));
	printf("eip:0x%X err:%d\n", r->eip, r->err_code);
	printf("cs:0x%X ds:0x%X es:0x%X gs:0x%X fs:0x%X ss:0x%X\n",r->cs,r->ds,r->es,r->gs,r->fs,r->ss);
	printf("eax:0x%X ebx:0x%X ecx:0x%X edx:0x%X\n",r->eax,r->ebx,r->ecx,r->edx);
	printf("edi: 0x%X esi: 0x%X ebp: 0x%X esp:0x%X\n",r->edi,r->esi,r->ebp,r->esp);
	printf("EFLAGS: 0x%X",r->eflags);
}
