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

#include <kernel/kstd/unix_types.h>
#include "TimeKeeper.h"
#include <kernel/kstd/circular_queue.hpp>

class TimeManager {
public:
	static void init();
	static TimeManager& inst();

	static timeval uptime();
	static timeval now();
	static double percent_idle();

protected:
	friend class TimeKeeper;
	void tick();

private:
	TimeManager();

	static TimeManager* _inst;
	TimeKeeper* _keeper = nullptr;
	timeval _epoch = {0, 0};
	timeval _uptime = {0, 0};
	int _ticks = 0;
	time_t _boot_epoch = 0;
	uint64_t _tsc_speed = 0; // Measured in MHz
	kstd::circular_queue<bool> idle_ticks = kstd::circular_queue<bool>(100);
};

