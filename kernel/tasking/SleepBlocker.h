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

#ifndef DUCKOS_SLEEPBLOCKER_H
#define DUCKOS_SLEEPBLOCKER_H

#include "Blocker.h"
#include <kernel/kstd/kstddef.h>
#include <kernel/time/Time.h>

class SleepBlocker: public Blocker {
public:
	explicit SleepBlocker(Time time);

	///Blocker
	bool is_ready() override;
	bool can_be_interrupted() override;

	///SleepBlocker
	Time end_time();
	Time time_left();

private:
	Time _end_time;
};


#endif //DUCKOS_SLEEPBLOCKER_H
