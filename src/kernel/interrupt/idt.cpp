#include <kstddef.h>
#include <interrupt/idt.h>

struct IDTEntry idt[256];
struct IDTPointer idtp;

void idt_set_gate(uint8_t num, uint32_t loc, uint16_t selector, uint8_t attrs){
	idt[num].offset_low = (loc & 0xFFFF);
	idt[num].offset_high = (loc >> 16) & 0xFFFF;
	idt[num].selector = selector;
	idt[num].zero = 0;
	idt[num].attrs = attrs;
}

void register_idt(){
	idtp.size = (sizeof(struct IDTEntry) * 256) - 1;
	idtp.offset = (int)&idt;
	
	memset(&idt, 0, sizeof(struct IDTEntry) * 256);
	
	idt_load();
}