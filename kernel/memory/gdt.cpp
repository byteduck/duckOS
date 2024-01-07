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
#include <kernel/memory/gdt.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/kstd/cstring.h>
#include <kernel/interrupt/isr.h>

Memory::GDTEntry gdt[GDT_ENTRIES];
Memory::GDTPointer gp;

extern "C" void* stack;

void Memory::gdt_set_gate(uint32_t num, uint32_t limit, uint32_t base, bool read_write, bool executable, bool type, uint8_t ring, bool present, bool accessed) {
	gdt[num].base_low = (base & 0xFFFFu);
	gdt[num].base_middle = (base >> 16u) & 0xFFu;
	gdt[num].base_high = (base >> 24u) & 0xFFu;

	gdt[num].limit_low = limit & 0xFFFFu;

	gdt[num].flags_and_limit.bits.limit_high = (limit >> 16u) & 0xFu;
	gdt[num].flags_and_limit.bits.zero = 0;
	gdt[num].flags_and_limit.bits.size = true; //32-bit
	gdt[num].flags_and_limit.bits.granularity = true; //4KiB pages

	gdt[num].access.bits.present = present;
	gdt[num].access.bits.accessed = accessed;
	gdt[num].access.bits.read_write = read_write;
	gdt[num].access.bits.executable = executable;
	gdt[num].access.bits.direction = false;
	gdt[num].access.bits.type = type;
	gdt[num].access.bits.ring = ring;
}

void Memory::setup_tss(int slot, TSS& tss){
	uint32_t base = (uint32_t) &tss;
	uint32_t limit = sizeof(tss);

	// Now, add our TSS descriptor's address to the GDT.
	gdt[slot].limit_low = limit & 0xFFFFu;
	gdt[slot].base_low = (base & 0xFFFFu);
	gdt[slot].base_middle = (base >> 16u) & 0xFFu;
	gdt[slot].base_high = (base >> 24u) & 0xFFu;
	gdt[slot].access.bits.accessed = true; //This indicates it's a TSS and not a LDT. This is a changed meaning
	gdt[slot].access.bits.read_write = false; //This indicates if the TSS is busy or not. 0 for not busy
	gdt[slot].access.bits.direction = false; //always 0 for TSS
	gdt[slot].access.bits.executable = true; //For TSS this is 1 for 32bit usage, or 0 for 16bit.
	gdt[slot].access.bits.type = false; //indicate it is a TSS
	gdt[slot].access.bits.ring = 0; //same meaning
	gdt[slot].access.bits.present = true; //same meaning
	gdt[slot].flags_and_limit.bits.limit_high = (limit >> 16u) & 0xFu; //isolate top nibble
	gdt[slot].flags_and_limit.bits.zero = 0;
	gdt[slot].flags_and_limit.bits.size = false; //should leave zero according to manuals. No effect
	gdt[slot].flags_and_limit.bits.granularity = false; //so that our computed GDT limit is in bytes, not pages
}

void Memory::load_gdt(){
	gp.limit = (sizeof(GDTEntry) * GDT_ENTRIES);
	gp.base = (uint32_t) &gdt;

	gdt_set_gate(0, 0, 0, false, false, false, 0, false); //Null
	gdt_set_gate(1, 0xFFFFF, 0, true, true, true, 0); //Kernel Code
	gdt_set_gate(2, 0xFFFFF, 0, true, false, true, 0); //Kernel Data
	gdt_set_gate(3, 0xFFFFF, 0, true, true, true, 3); //User code
	gdt_set_gate(4, 0xFFFFF, 0, true, false, true, 3); //User data

	setup_tss(5, TaskManager::tss);
	setup_tss(6, Interrupt::fault_tss);

	gdt_flush();
	asm volatile("ltr %0": : "r"((uint16_t)0x28));
}
