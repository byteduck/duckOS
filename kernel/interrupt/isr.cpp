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
#include <kernel/memory/paging.h>
#include <kernel/kstdio.h>
#include <kernel/interrupt/idt.h>
#include <kernel/interrupt/isr.h>
#include <kernel/tasking/TaskManager.h>

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

bool fpanic(char *a, char *b, uint32_t sig){
	if(!TaskManager::enabled() || TaskManager::current_process()->pid() == 1){
		cli();
		PANIC(a,b,false);
		return true;
	}else{
		TaskManager::notify(sig);
		return false;
	}
}

void fault_handler(struct Registers *r){
	if(r->num < 32){
		switch(r->num){
			case 0:
			if(fpanic("DIVIDE_BY_ZERO", "Instruction pointer:", SIGILL)){
				printHexl(r->err_code);
				while(true);
			}
			break;

			case 13: //GPF
			if(fpanic("GENERAL_PROTECTION_FAULT", "Instruction pointer, error code, and Registers:", SIGILL)){
				print_regs(r);
				while(true);
			}
			break;

			case 14: //Page fault
			if(TaskManager::current_process() == nullptr || TaskManager::current_process()->pid() == 1) {
				Paging::page_fault_handler(r);
			} else {
				TaskManager::notify(SIGSEGV);
			}
			break;

			default:
			if(fpanic("Something weird happened.", "Fault and Instruction pointer:", SIGILL)){
				printHex(r->num);
				print(" and ");
				printHexl(r->err_code);
				while(true);
			}
			break;
		}
	}
}

void print_regs(struct Registers *r){
	asm volatile("mov %%ss, %%eax":"=a"(r->ss));
	printf("eip:0x%X err:%d\n", r->eip, r->err_code);
	printf("cs:0x%X ds:0x%X es:0x%X gs:0x%X fs:0x%X ss:0x%X\n",r->cs,r->ds,r->es,r->gs,r->fs,r->ss);
	printf("eax:0x%X ebx:0x%X ecx:0x%X edx:0x%X\n",r->eax,r->ebx,r->ecx,r->edx);
	printf("edi: 0x%X esi: 0x%X ebp: 0x%X\n",r->edi,r->esi,r->ebp);
	printf("EFLAGS: 0x%X",r->eflags);
}
