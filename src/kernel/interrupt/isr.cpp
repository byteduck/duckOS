#include <common.h>
#include <memory/paging.h>
#include <stdio.h>
#include <interrupt/idt.h>
#include <interrupt/isr.h>
#include <tasking/tasking.h>

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
	if(getCurrentProcess()->pid == 1){
		cli();
		PANIC(a,b,false);
		return true;
	}else{
		notify(sig);
		return false;
	}
}

void fault_handler(struct registers *r){
	if(r->num < 32){
		switch(r->num){
			case 0:
			if(fpanic("DIVIDE_BY_ZERO", "Instruction pointer:", SIGILL)){
				printHexl(r->err_code);
				while(true);
			}
			break;

			case 13: //GPF
			if(fpanic("GENERAL_PROTECTION_FAULT", "Instruction pointer, error code, and registers:", SIGILL)){
				print_regs(r);
				while(true);
			}
			break;

			case 14: //Page fault
			if(getCurrentProcess()->pid == 1)
				pageFaultHandler(r);
			else
				notify(SIGILL);
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

void print_regs(struct registers *r){
	asm volatile("mov %%ss, %%eax":"=a"(r->ss));
	printf("eip:0x%X err:%d\n", r->eip, r->err_code);
	printf("cs:0x%X ds:0x%X es:0x%X gs:0x%X fs:0x%X ss:0x%X\n",r->cs,r->ds,r->es,r->gs,r->fs,r->ss);
	printf("e?x: a:0x%X b:0x%X c:0x%X d:0x%X\n",r->eax,r->ebx,r->ecx,r->edx);
	printf("edi: 0x%X esi: 0x%X ebp: 0x%X\n",r->edi,r->esi,r->ebp);
	printf("EFLAGS: 0x%X",r->eflags);
}
