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

#ifndef DUCKOS_CMOS_H
#define DUCKOS_CMOS_H

//Converts binary coded decimal (bcd) to normal numbers
#include <common/cstddef.h>
#include "kstddef.h"

#define bcd(val) ((val / 16) * 10 + (val & 0xf))

#define	CMOS_ADDRESS 0x70
#define	CMOS_DATA 0x71

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

#define CMOS_STATUS_UPDATE_IN_PROGRESS  0x80

namespace CMOS {
	typedef struct CMOSTime {
		uint8_t seconds;
		uint8_t minutes;
		uint8_t hours;
		uint8_t weekday;
		uint8_t day;
		uint8_t month;
		uint16_t year;
	} CMOSTime;

	void get_time(CMOSTime& time) {
		outb(CMOS_ADDRESS, CMOS_SECONDS);
		time.seconds = inb(CMOS_DATA);
		outb(CMOS_ADDRESS, CMOS_MINUTES);
		time.minutes = inb(CMOS_DATA);
		outb(CMOS_ADDRESS, CMOS_HOURS);
		time.hours = inb(CMOS_DATA);
		outb(CMOS_ADDRESS, CMOS_WEEKDAY);
		time.weekday = inb(CMOS_DATA);
		outb(CMOS_ADDRESS, CMOS_DAY);
		time.day = inb(CMOS_DATA);
		outb(CMOS_ADDRESS, CMOS_MONTH);
		time.month = inb(CMOS_DATA);
		outb(CMOS_ADDRESS, CMOS_YEAR);
		time.year = inb(CMOS_DATA);
		outb(CMOS_ADDRESS, CMOS_CENTURY);
		time.year += inb(CMOS_DATA) * 100;
	}
};

#endif //DUCKOS_CMOS_H
