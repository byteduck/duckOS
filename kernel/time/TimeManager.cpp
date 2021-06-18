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

#include <kernel/tasking/TaskManager.h>
#include "TimeManager.h"
#include "PIT.h"
#include "RTC.h"

TimeManager* TimeManager::_inst = nullptr;

void TimeManager::init() {
	if(_inst)
		return;
	_inst = new TimeManager();
	_inst->_keeper->enable();
}

TimeManager::TimeManager(): _keeper(new RTC(this)) {
	_epoch.tv_sec = RTC::timestamp();
}

TimeManager& TimeManager::inst() {
	return *_inst;
}

long int TimeManager::uptime() {
	return _inst->_uptime;
}

timespec TimeManager::now() {
	return _inst->_epoch;
}

void TimeManager::tick() {
	_ticks++;

	if(_ticks % (_keeper->frequency() / 64) == 0) {
		if(idle_ticks.size() == 100)
			idle_ticks.pop_front();
		idle_ticks.push(TaskManager::is_idle());
		TaskManager::preempt();
	}

	if(_ticks == _keeper->frequency()) {
		_ticks = 0;
		_uptime++;
		_epoch.tv_sec++;
		_epoch.tv_usec = 0;
	}

	_epoch.tv_usec += 1000000 / _keeper->frequency();
}

double TimeManager::percent_idle() {
	bool* ticks_storage = _inst->idle_ticks.storage();
	int num_idle = 0;
	for(size_t i = 0; i < 100; i++)
		if(ticks_storage[i])
			num_idle++;
	return (double) num_idle / 100.0;
}