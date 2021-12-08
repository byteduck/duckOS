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

#include "SleepBlocker.h"
#include <kernel/kstd/kstdio.h>

SleepBlocker::SleepBlocker(Time time): _end_time(Time::now() + time) {
}

bool SleepBlocker::is_ready() {
	return Time::now() >= _end_time;
}

bool SleepBlocker::can_be_interrupted() {
	return true;
}

Time SleepBlocker::end_time() {
	return _end_time;
}

Time SleepBlocker::time_left() {
	return was_interrupted() ? _end_time - Time::now() : Time();
}