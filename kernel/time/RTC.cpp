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

#include "RTC.h"
#include "CMOS.h"
#include "../tasking/TaskManager.h"
#include <kernel/interrupt/interrupt.h>

#define LEAPYEAR(year) (((year) % 4 == 0) && (((year) % 100 != 0) || ((year) % 400 == 0)))

#define SECSPERDAY 86400
#define SECSPERHOUR 3600
#define SECSPERMIN 60
#define EPOCH 1970

const int days_per_month[12] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

RTC::RTC(TimeManager* time): IRQHandler(RTC_IRQ), TimeKeeper(time) {
}

time_t RTC::timestamp() {
	while(CMOS::read(CMOS_STATUS_A) & CMOS_STATUS_UPDATE_IN_PROGRESS) {}

	int second = bcd(CMOS::read(CMOS_SECONDS));
	int minute = bcd(CMOS::read(CMOS_MINUTES));
	int hour = bcd(CMOS::read(CMOS_HOURS));
	int day = bcd(CMOS::read(CMOS_DAY));
	int month = bcd(CMOS::read(CMOS_MONTH));
	int years = bcd(CMOS::read(CMOS_YEAR));
	years += bcd(CMOS::read(CMOS_CENTURY)) * 100;
	years -= EPOCH;

	//Years
	int secs = years * (SECSPERDAY * 365);

	//Leap years
	int num_leap = 0;
	for (int i = 0; i < (years - 1); i++) {
		if (LEAPYEAR((EPOCH + i)))
			num_leap++;
	}
	secs += (num_leap * SECSPERDAY);

	//Seconds, hours, minutes, days
	secs += second;
	secs += (hour * SECSPERHOUR);
	secs += (minute * SECSPERMIN);
	secs += (day * SECSPERDAY);

	//Months
	for(int i = 0; i < month - 1; i++) {
		secs += (days_per_month[i] * SECSPERDAY);
	}
	if(LEAPYEAR(years))
		secs += SECSPERDAY;

	return secs;
}

void RTC::handle_irq(Registers* regs) {
	CMOS::read(0x8C);
	TimeKeeper::tick();
}

bool RTC::set_frequency(int frequency) {
	if(frequency <= 0)
		return false;
	if(RTC_FREQUENCY_DIVIDER % frequency)
		return false;

	//Take log_2(RTC_FREQUENCY_DIVIDER / frequency) by finding the position of the highest bit
	int divided_freq = RTC_FREQUENCY_DIVIDER / frequency;
	uint8_t highest_bit = 1;
	while(divided_freq >>= 1)
		highest_bit++;

	//Check if it's valid and write the frequency
	if(highest_bit < RTC_FREQUENCYVAL_MIN || highest_bit > RTC_FREQUENCYVAL_MAX)
		return false;

	IRQHandler::uninstall_irq();
	_frequency = frequency;
	CMOS::write(0x8A, highest_bit | (CMOS::read(0x8A) & 0xF0));
	IRQHandler::reinstall_irq();

	return true;
}

int RTC::frequency() {
	return _frequency;
}

void RTC::enable() {
	//Disable interrupts and non-maskable interrupts and enable the RTC update ended interrupt
	TaskManager::ScopedCritical critical;
	Interrupt::NMIDisabler nmidis;
	CMOS::write(0x8B, CMOS_SQUARE_WAVE_INTERRUPT_FLAG | CMOS::read(CMOS_STATUS_B));
	set_frequency(RTC_FREQUENCY);
}

void RTC::disable() {
	//Disable interrupts and non-maskable interrupts and enable the RTC update ended interrupt
	TaskManager::ScopedCritical critical;
	Interrupt::NMIDisabler nmidis;
	CMOS::write(0x8B, CMOS::read(CMOS_STATUS_B) & (~CMOS_SQUARE_WAVE_INTERRUPT_FLAG));
}

bool RTC::mark_in_irq() {
	return true;
}
