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

#include "SpinLock.h"
#include "Thread.h"
#include "TaskManager.h"
#include <kernel/Atomic.h>

SpinLock::SpinLock() = default;

SpinLock::~SpinLock() = default;

bool SpinLock::locked() {
	return _times_locked.load(MemoryOrder::SeqCst);
}

void SpinLock::release() {
	if(!TaskManager::enabled())
		return;

	// Decrease counter. If the counter is zero, release the lock
	if(_times_locked.sub(1, MemoryOrder::Release) == 1) {
		_blocker.set_ready(true);
	}
}

void SpinLock::acquire() {
	auto cur_thread = TaskManager::current_thread();
	if(!TaskManager::enabled() || !cur_thread) return; //Tasking isn't initialized yet

	//Loop while the lock is held
	int expected = 0;
	while(!_times_locked.compare_exchange_strong(expected, 1)) {
		expected = 0;
		if(_holding_thread == cur_thread) {
			// TODO: This is definitely susceptible to a race condition. Figure out how to make a recursive lock that works
			//We are the holding process, so increment the counter and return
			_blocker.set_ready(false);
			_times_locked.add(1);
			return;
		}
		//TODO: Find a good way to unblock just one waiting task
		cur_thread->block(_blocker);
	}

	//We now hold the lock
	_blocker.set_ready(false);
	_holding_thread = cur_thread;
}