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
#include <kernel/kstd/KLog.h>

#if defined(__i386__)
#include "kernel/arch/i386/time/PIT.h"
#include "kernel/arch/i386/time/RTC.h"
#elif defined(__aarch64__)
#include <kernel/arch/aarch64/ARMTimer.h>
#endif

TimeManager* TimeManager::_inst = nullptr;

#if defined(__i386__)
inline uint64_t read_tsc() {
	uint32_t low, high;
	asm volatile ("rdtsc" : "=a"(low), "=d"(high));
	return ((uint64_t) high << 32) | (uint64_t) low;
	return 0;
}

extern uint64_t initial_tsc;
extern uint64_t final_tsc;
extern "C" void __attribute((cdecl)) measure_tsc_speed();
#endif

void TimeManager::init() {
	if(_inst)
		return;

	_inst = new TimeManager();
	_inst->_keeper->enable();
}

TimeManager::TimeManager() {
#if defined(__i386__)
	_keeper = new RTC(this);
	_boot_epoch = RTC::timestamp();

	// TODO: aarch64
	// Measure the tsc speed in MHz for accurate time measurement by using the PIT.
	measure_tsc_speed();
	_tsc_speed = (final_tsc - initial_tsc) / 10000;
#elif defined(__aarch64__)
	_keeper = new ARMTimer(this);
	_boot_epoch = 0; // TODO: aarch64
	_tsc_speed = 1;
#endif

	KLog::dbg("TimeManager", "TSC speed measured at {}MHz", (uint32_t) _tsc_speed);
}

TimeManager& TimeManager::inst() {
	return *_inst;
}

timeval TimeManager::uptime() {
	if(!_inst)
		return {0, 0};
	return _inst->_uptime;
}

timeval TimeManager::now() {
	return _inst->_epoch;
}

void TimeManager::tick() {
	_ticks++;

	if(idle_ticks.size() == 100)
		idle_ticks.pop_front();
	idle_ticks.push_back(TaskManager::is_idle());
	TaskManager::tick();

#if defined(__i386__)
	auto uptime_us = (read_tsc() - initial_tsc) / _tsc_speed;
#elif defined(__aarch64__)
	long uptime_us = 0; // TODO: aarch64
#endif
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