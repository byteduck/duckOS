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

#ifndef DUCKOS_INTERRUPT_H
#define DUCKOS_INTERRUPT_H

#include <kernel/IO.h>

#define CMOS_PORT 0x70
#define NMI_FLAG 0x80

namespace Interrupt {
	void init();

	class Disabler {
	public:
		inline Disabler() {
			asm volatile("cli");
		}

		inline ~Disabler() {
			asm volatile("sti");
		}
	};

	class NMIDisabler {
	public:
		inline NMIDisabler() {
			IO::outb(CMOS_PORT, NMI_FLAG | IO::inb(CMOS_PORT));
		}

		inline ~NMIDisabler() {
			IO::outb(CMOS_PORT, IO::inb(CMOS_PORT) & (~NMI_FLAG));
		}
	};
}

#endif //DUCKOS_INTERRUPT_H
