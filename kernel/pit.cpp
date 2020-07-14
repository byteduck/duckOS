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

#include <kernel/kstddef.h>
#include <kernel/pit.h>
#include <kernel/tasking/TaskManager.h>

namespace PIT {
	uint32_t ticks = 0;
	uint32_t seconds = 0;

	void pit_handler(){
		if(++ticks == 1000) {
			ticks = 0;
			seconds++;
		}
		TaskManager::preempt();
	}

	uint32_t get_seconds() {
		return seconds;
	}

	uint32_t get_nseconds() {
		return (long)ticks * 1000;
	}

	void gettimeofday(struct timespec *t, void *tz) {
		t->tv_sec = seconds;
		t->tv_nsec = (long)ticks * 1000;
	}

	static inline void pit_send_data(uint16_t data, uint8_t counter){
		uint8_t port = (counter==0) ? PIT_COUNTER0 : ((counter==1) ? PIT_COUNTER1 : PIT_COUNTER2);
		outb(port, (uint8_t)data);
	}

	void init(){
		auto divisor = (uint16_t)( 1193180u / PIT_FREQUENCY);

		uint8_t ocw = 0;
		ocw = (ocw & ~0xEu) | 0x6u;
		ocw = (ocw & ~0x30u) | 0x30u;
		ocw = (ocw & ~0xC0u) | 0u;
		outb(PIT_CMD, ocw);

		// set frequency rate
		pit_send_data(divisor & 0xffu, 0);
		pit_send_data((divisor >> 8u) & 0xffu, 0);
	}
}
