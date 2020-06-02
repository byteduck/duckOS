#include <common.h>
#include <interrupt/idt.h>
#include <interrupt/irq.h>
#include <pit.h>

void (*irq_routines[16])(struct registers *r) = {
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};

void irq_add_handler(int irq, void (*handler)(struct registers *r)){
	irq_routines[irq] = handler;
}

void irq_remove_handler(int irq){
	irq_routines[irq] = 0;
}

void irq_remap(){
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
    	outb(0xA1, 0x02);
    	outb(0x21, 0x01);
    	outb(0xA1, 0x01);
    	outb(0x21, 0x0);
    	outb(0xA1, 0x0);
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

void irq_handler(struct registers *r){
	void (*handler)(struct registers *r);

	handler = irq_routines[r->num - 32];
	if(handler /*If it is not 0, AKA it is registered*/){
		handler(r);
	}

	if(r->num >= 40){
		outb(0xA0, 0x20); //If it is greater than 40, send an end of interrupt to slave controller
	}

	outb(0x20, 0x20); //Send EOI to controller
}
