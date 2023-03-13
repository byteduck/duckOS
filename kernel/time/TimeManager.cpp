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

#include <kernel/tasking/TaskManager.h>
#include "TimeManager.h"
#include "PIT.h"
#include "RTC.h"
#include <kernel/kstd/KLog.h>

TimeManager* TimeManager::_inst = nullptr;

inline uint64_t read_tsc() {
	uint32_t low, high;
	asm volatile ("rdtsc" : "=a"(low), "=d"(high));
	return ((uint64_t) high << 32) | (uint64_t) low;
}

extern uint64_t initial_tsc;
extern uint64_t final_tsc;

void TimeManager::init() {
	if(_inst)
		return;

	_inst = new TimeManager();
	_inst->_keeper->enable();
}

TimeManager::TimeManager(): _keeper(new RTC(this)) {
	// Measure the tsc speed in MHz for accurate time measurement by using the PIT.
	_boot_epoch = RTC::timestamp();
	measure_tsc_speed();
	_tsc_speed = (final_tsc - initial_tsc) / 10000;
	KLog::dbg("TimeManager", "TSC speed measured at %dMHz", (uint32_t) _tsc_speed);
}

TimeManager& TimeManager::inst() {
	return *_inst;
}

timespec TimeManager::uptime() {
	if(!_inst)
		return {0, 0};
	return _inst->_uptime;
}

timespec TimeManager::now() {
	return _inst->_epoch;
}

void TimeManager::tick() {
	_ticks++;

	if(idle_ticks.size() == 100)
		idle_ticks.pop_front();
	idle_ticks.push_back(TaskManager::is_idle());
	TaskManager::tick();

	auto uptime_us = (read_tsc() - initial_tsc) / _tsc_speed;
	_uptime.tv_usec = (long) (uptime_us % 1000000);
	_uptime.tv_sec = (long) (uptime_us / 1000000);
	_epoch.tv_sec = _boot_epoch + _uptime.tv_sec;
	_epoch.tv_usec = _uptime.tv_usec;
}

double TimeManager::percent_idle() {
	bool* ticks_storage = _inst->idle_ticks.storage();
	int num_idle = 0;
	for(size_t i = 0; i < 100; i++)
		if(ticks_storage[i])
			num_idle++;
	return (double) num_idle / 100.0;
}