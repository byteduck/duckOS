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

#ifndef IRQ_H
#define IRQ_H

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)

#include "IRQHandler.h"

namespace Interrupt {
	extern "C" void irq0();
	extern "C" void irq1();
	extern "C" void irq2();
	extern "C" void irq3();
	extern "C" void irq4();
	extern "C" void irq5();
	extern "C" void irq6();
	extern "C" void irq7();
	extern "C" void irq8();
	extern "C" void irq9();
	extern "C" void irq10();
	extern "C" void irq11();
	extern "C" void irq12();
	extern "C" void irq13();
	extern "C" void irq14();
	extern "C" void irq15();
	extern "C" void irq_handler(struct Registers *r);

	void irq_set_handler(int irq, IRQHandler *handler);
	void irq_remove_handler(int irq);
	void irq_remap();
	void irq_init();
	bool in_interrupt();
}

#endif
