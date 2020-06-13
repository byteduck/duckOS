#ifndef IDT_H
#define IDT_H

struct IDTEntry{
	unsigned short offset_low;  //Offset bits 0-15
	unsigned short selector;    //A code segment selector in the GDT
	unsigned char zero;         //Always 0
	unsigned char attrs;        //Type & Attributes
	unsigned short offset_high; //Offset bits 16-31
}__attribute__((packed));

struct IDTPointer{
	unsigned short size;
	unsigned int offset;
}__attribute__((packed));

extern "C" void idt_load();

void idt_set_gate(uint8_t num, uint32_t loc, uint16_t selector, uint8_t flags);
void register_idt();

#endif