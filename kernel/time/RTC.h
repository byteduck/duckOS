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

#ifndef DUCKOS_RTC_H
#define DUCKOS_RTC_H

//Converts binary coded decimal (bcd) to normal numbers
#include <kernel/kstd/unix_types.h>
#include <kernel/interrupt/IRQHandler.h>
#include "kernel/kstd/kstddef.h"
#include "TimeKeeper.h"

#define RTC_IRQ 0x8

#define CMOS_SECONDS 0x0
#define CMOS_MINUTES 0x2
#define CMOS_HOURS 0x4
#define CMOS_WEEKDAY 0x6
#define CMOS_DAY 0x7
#define CMOS_MONTH 0x8
#define CMOS_YEAR 0x9
#define CMOS_CENTURY 0x32
#define CMOS_STATUS_A 0x0A
#define CMOS_STATUS_B 0x0B
#define CMOS_STATUS_C 0x0C
#define CMOS_STATUS_UPDATE_IN_PROGRESS  0x80
#define CMOS_SQUARE_WAVE_INTERRUPT_FLAG 0x40

#define RTC_FREQUENCY 1024
#define RTC_FREQUENCYVAL_MIN 2
#define RTC_FREQUENCYVAL_MAX 14
#define RTC_FREQUENCY_DIVIDER 32768

#define bcd(val) ((val / 16) * 10 + (val & 0xf))

class RTC: public IRQHandler, public TimeKeeper {
public:
	///RTC
	RTC(TimeManager* time);
	static time_t timestamp();

	///IRQHandler
	void handle_irq(Registers* regs) override;
	bool mark_in_irq() override;

	///TimeHandler
	int frequency() override;
	void enable() override;
	void disable() override;

private:
	bool set_frequency(int frequency);

	int _timestamp = 0;
	int _frequency = 0;
};

#endif //DUCKOS_RTC_H
