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

#pragma once

#define PIT_COUNTER0 0x40
#define PIT_COUNTER1 0x41
#define PIT_COUNTER2 0x42
#define PIT_CMD  0x43
#define PIT_IRQ 0
#define PIT_FREQUENCY 1000 //Hz

#include "kernel/interrupt/IRQHandler.h"
#include "kernel/time/TimeKeeper.h"

class PIT: public TimeKeeper, public IRQHandler {
public:
	///PIT
	PIT(TimeManager* manager);

	///IRQHandler
	void handle_irq(IRQRegisters* regs) override;
	bool mark_in_irq() override;

	///TimeHandler
	int frequency() override;
	void enable() override;
	void disable() override;

private:
	static void write(uint16_t data, uint8_t counter);
};
