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

#include "IO.h"
#include "kernel/kstd/kstdio.h"
#include "kernel/memory/MemoryManager.h"

#if defined(__i386__)
void IO::wait() {
	asm volatile ( "jmp 1f\n\t"
				   "1:jmp 2f\n\t"
				   "2:" );
}

void IO::outb(uint16_t port, uint8_t value){
	asm volatile ("outb %1, %0" : : "d" (port), "a" (value));
}

void IO::outw(uint16_t port, uint16_t value){
	asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

void IO::outl(uint16_t port, uint32_t value){
	asm volatile ("outl %1, %0" : : "dN" (port), "a" (value));
}

uint8_t IO::inb(uint16_t port){
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint16_t IO::inw(uint16_t port){
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint32_t IO::inl(uint16_t port){
	uint32_t ret;
	asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}
#elif defined(__aarch64__)
void IO::wait() {
	// TODO: aarch64
}
#endif

IO::Window::Window(PCI::Address addr, uint8_t bar) {
	auto bar_val = PCI::read_dword(addr, bar);
	if (!(bar_val & 0x1u)) {
		// Memory IO
		auto type = (bar_val >> 1) & 0x3u;
		m_prefetchable = bar_val & 0x8u;
		switch (type) {
		case 0:
			m_type = Mem32;
			m_addr = bar_val & ~0xFu;
			break;
		default:
			ASSERT(false);
		}

		PCI::write_dword(addr, bar, 0xFFFFFFFF);
		m_size = ~(PCI::read_dword(addr, bar) & (~0xfull)) + 1;
		PCI::write_dword(addr, bar, bar_val);
		m_vm_region = MM.map_device_region(m_addr, m_size);
	} else {
#if defined(__i386__)
		m_type = IOSpace;
		m_addr = bar_val & ~0x3u;
#else
		m_type = Invalid;
#endif
	}
}
