#include <kernel/kstddef.h>
#include <kernel/memory/gdt.h>

GDTEntry gdt[GDT_ENTRIES];
GDTPointer gp;

extern "C" void gdt_flush();

void gdt_set_gate(uint32_t num, uint16_t limit, uint32_t base, uint8_t access, uint8_t gran){
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit = limit;
    gdt[num].granularity = gran;

    gdt[num].access = access;
}
void load_gdt(){
	gp.limit = (sizeof(GDTEntry) * GDT_ENTRIES) - 1;
	gp.base = (uint32_t)&gdt;

	gdt_set_gate(0, 0, 0, 0, 0); //Null
	gdt_set_gate(1, 0xFFFF, 0, 0b10011010, 0b11001111); //Kernel Code
	gdt_set_gate(2, 0xFFFF, 0, 0b10010010, 0b11001111); //Kernel Data
	gdt_set_gate(3, 0xFFFF, 0, 0b11111010, 0b11001111); //User code
	gdt_set_gate(4, 0xFFFF, 0, 0b11110010, 0b11001111); //User data

	asm volatile("lgdt %0": "=m"(gp));
}
