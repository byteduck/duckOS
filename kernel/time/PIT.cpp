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

#include <kernel/kstd/kstddef.h>
#include <kernel/time/PIT.h>
#include <kernel/IO.h>
#include "TimeManager.h"

PIT::PIT(TimeManager* manager): TimeKeeper(manager), IRQHandler(PIT_IRQ) {
	auto divisor = (uint16_t)(1193180u / PIT_FREQUENCY);

	uint8_t ocw = 0;
	ocw = (ocw & ~0xEu) | 0x6u;
	ocw = (ocw & ~0x30u) | 0x30u;
	ocw = (ocw & ~0xC0u) | 0u;
	IO::outb(PIT_CMD, ocw);

	// set frequency rate
	write(divisor & 0xffu, 0);
	write((divisor >> 8u) & 0xffu, 0);
}

void PIT::handle_irq(Registers* regs) {
	TimeKeeper::tick();
}

bool PIT::mark_in_irq() {
	return false;
}

int PIT::frequency() {
	return PIT_FREQUENCY;
}

void PIT::enable() {
    //TODO
}

void PIT::disable() {
    //TODO
}

void PIT::write(uint16_t data, uint8_t counter){
	uint8_t port = (counter==0) ? PIT_COUNTER0 : ((counter==1) ? PIT_COUNTER1 : PIT_COUNTER2);
	IO::outb(port, (uint8_t)data);
}
