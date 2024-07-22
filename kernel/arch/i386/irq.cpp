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

#include <kernel/kstd/kstddef.h>
#include "idt.h"
#include "irq.h"
#include <kernel/tasking/TaskManager.h>
#include <kernel/interrupt/IRQHandler.h>
#include <kernel/interrupt/interrupt.h>

namespace Interrupt {
	IRQHandler* handlers[16] = {nullptr};

	volatile bool _in_interrupt = false;

	void irq_set_handler(int irq, IRQHandler* handler){
		handlers[irq] = handler;
	}

	void irq_remove_handler(int irq){
		handlers[irq] = nullptr;
	}

	void irq_remap(){
		IO::outb(PIC1_COMMAND, 0x11); //Init
		IO::wait();
		IO::outb(PIC2_COMMAND, 0x11); //Init
		IO::wait();
		IO::outb(PIC1_DATA, 0x20); //Offset
		IO::wait();
		IO::outb(PIC2_DATA, 0x28); //Offset
		IO::wait();
		IO::outb(PIC1_DATA, 0x04); //Tell master PIC slave PIC is at irq2
		IO::wait();
		IO::outb(PIC2_DATA, 0x02); //Slave PIC cascade identity
		IO::wait();
		IO::outb(PIC1_DATA, 0x01); //8086 mode
		IO::wait();
		IO::outb(PIC2_DATA, 0x01); //8086 mode
		IO::wait();

		//No mask
		IO::outb(PIC1_DATA, 0);
		IO::outb(PIC2_DATA, 0);
	}

	void irq_init(){
		irq_remap();
		idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
		idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
		idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
		idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
		idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
		idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
		idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
		idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);
		idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
		idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
		idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
		idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
		idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
		idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
		idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
		idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);
	}

	void irq_handler(IRQRegisters* regs){
		{
			auto thread = TaskManager::current_thread();
			TrapFrame frame { nullptr, TrapFrame::IRQ, regs };
			if (thread)
				thread->enter_trap_frame(&frame);
		}

		regs->irq_num -= 0x20;
		if (regs->irq_num >= (sizeof(handlers) / sizeof(handlers[0])))
			PANIC("INVALID_IRQ", "Attempted to handle invalid IRQ %d", regs->irq_num);
		auto handler = handlers[regs->irq_num];
		if(handler) {
			//Mark that we're in an interrupt so that yield will be async if it occurs
			_in_interrupt = handler->mark_in_irq();

			//Handle the IRQ
			handler->handle(regs);
		}

		//Send EOI if we haven't already
		if(!handler || !handler->sent_eoi())
			send_eoi(regs->irq_num);

		//If we need to yield asynchronously after the interrupt because we called TaskManager::yield() during it, do so
		TaskManager::do_yield_async();

		{
			auto thread = TaskManager::current_thread();
			if (thread)
				thread->exit_trap_frame();
		}
	}

	bool in_irq() {
		return _in_interrupt;
	}

	void send_eoi(int irq_number) {
		if(irq_number >= 8)
			IO::outb(PIC2_COMMAND, 0x20);
		IO::outb(PIC1_COMMAND, 0x20);
		_in_interrupt = false;
	}
}
