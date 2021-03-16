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

#include "SleepBlocker.h"
#include <kernel/pit.h>

SleepBlocker::SleepBlocker(unsigned int seconds): _end_time(PIT::get_mseconds() + (uint32_t)seconds * 1000) {

}

bool SleepBlocker::is_ready() {
	return PIT::get_mseconds() >= _end_time;
}

bool SleepBlocker::can_be_interrupted() {
	return true;
}

int SleepBlocker::end_time() {
	return (int)(_end_time / 1000);
}

int SleepBlocker::time_left() {
	auto msecs = PIT::get_mseconds();
	if(msecs > _end_time)
		return 0;
	else
		return (int)(((long)_end_time - (long)msecs) / 1000);
}