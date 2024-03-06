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
#include <kernel/KernelMapper.h>
#include <kernel/arch/i386/registers.h>

namespace Interrupt {
	TSS fault_tss;

	[[noreturn]] void double_fault();

	void isr_init(){
		idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
		idt_set_gate(1, (unsigned)isr1, 0x08, 0x8E);
		idt_set_gate(2, (unsigned)isr2, 0x08, 0x8E);
		idt_set_gate(3, (unsigned)isr3, 0x08, 0x8E);
		idt_set_gate(4, (unsigned)isr4, 0x08, 0x8E);
		idt_set_gate(5, (unsigned)isr5, 0x08, 0x8E);
		idt_set_gate(6, (unsigned)isr6, 0x08, 0x8E);
		idt_set_gate(7, (unsigned)isr7, 0x08, 0x8E);
		// Special case for double-fault; we want to use a separate TSS so we can be sure we have a clean stack to work with.
		idt_set_gate(8, 0, 0x30, 0x85);
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

		// Setup the double-fault TSS and a stack for it
		memset(&fault_tss, 0, sizeof(TSS));	
		fault_tss.ss0 = 0x10;
		fault_tss.cs = 0x08;
		fault_tss.ss = 0x10;
		fault_tss.ds = 0x10;
		fault_tss.es = 0x10;
		fault_tss.fs = 0x10;
		fault_tss.gs = 0x10;
		fault_tss.ss = 0x10;
		fault_tss.eflags = 0x2;
		fault_tss.cr3 = MM.kernel_page_directory.entries_physaddr();
		fault_tss.esp0 = MM.inst().alloc_kernel_stack_region(PAGE_SIZE * 2)->end();
		fault_tss.esp = fault_tss.esp0;
		fault_tss.eip = (size_t) double_fault;
	}

	[[noreturn]] void double_fault() {
		PANIC_NOHLT("DOUBLE_FAULT", "A double fault occurred. Something has gone horribly wrong.");
		if (!MM.kernel_page_directory.is_mapped(TaskManager::tss.esp + sizeof(void*), false)) {
			printf("Looks like a stack overflow occurred in the kernel. Hold on, this is gonna be a doozy:\n");
		}
		KernelMapper::print_stacktrace(TaskManager::tss.ebp);
		asm volatile("cli; hlt");
		while(1);
	}

	void handle_fault(const char* err, const char* panic_msg, uint32_t sig, ISRRegisters* regs) {
		if(!TaskManager::enabled() || TaskManager::current_thread()->is_kernel_mode() || TaskManager::is_preempting()) {
			PANIC(err, "%s\nFault %d at 0x%x", panic_msg, regs->isr_num, regs->interrupt_frame.eip);
		} else {
			TaskManager::current_process()->kill(sig);
		}
	}

	void fault_handler(ISRRegisters* regs){
		if(regs->isr_num < 32){
			switch(regs->isr_num){
				case 0:
					handle_fault("DIVIDE_BY_ZERO", "Please don't do that.", SIGILL, regs);
					break;

				case 13: //GPF
					handle_fault("GENERAL_PROTECTION_FAULT", "How did you manage to do that?", SIGILL, regs);
					break;

				case 14: //Page fault
				{
					size_t err_pos;
					asm volatile ("mov %%cr2, %0" : "=r" (err_pos));
					PageFault::Type type;
					switch (regs->err_code) {
						case FAULT_USER_READ:
						case FAULT_USER_READ_GPF:
						case FAULT_KERNEL_READ:
						case FAULT_KERNEL_READ_GPF:
							type = PageFault::Type::Read;
							break;
						case FAULT_USER_WRITE:
						case FAULT_USER_WRITE_GPF:
						case FAULT_KERNEL_WRITE:
						case FAULT_KERNEL_WRITE_GPF:
							type = PageFault::Type::Write;
							break;
						default:
							type = PageFault::Type::Unknown;
					}
					const PageFault fault { err_pos, regs, type };
					if(TaskManager::is_preempting() || fault.type == PageFault::Type::Unknown || !TaskManager::current_thread()) {
						// Never want to fault while preempting
						MemoryManager::inst().page_fault_handler(regs);
					} else {
						TaskManager::current_thread()->handle_pagefault(fault);
					}
					break;
				}

				default:
					handle_fault("UNKNOWN_FAULT", "What did you do?", SIGILL, regs);
			}
		}
	}
}
