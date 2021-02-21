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
#include "TaskManager.h"
#include <kernel/Atomic.h>

SpinLock::SpinLock() = default;

SpinLock::~SpinLock() = default;

bool SpinLock::locked() {
	return _locked;
}

void SpinLock::release() {
	if(!TaskManager::enabled())
		return;

	//Decrease counter
	Atomic::dec(&_times_locked);

	//If the counter is zero, release the lock
	if(_times_locked == 0) {
		_holding_process = nullptr;
		Atomic::store(&_locked, 0);
		_blocker.set_ready(true);
	}
}

void SpinLock::acquire() {
	auto* cur_proc = TaskManager::current_process();
	if(!TaskManager::enabled() || !cur_proc) return; //Tasking isn't initialized yet

	//Loop while the lock is held
	while(Atomic::swap(&_locked, 1)) {
		if(_holding_process == cur_proc) {
			//We are the holding process, so increment the counter and return
			_blocker.set_ready(false);
			Atomic::inc(&_times_locked);
			return;
		}
		//TODO: Find a good way to unblock just one waiting task
		cur_proc->block(_blocker);
	}

	//We now hold the lock
	_blocker.set_ready(false);
	Atomic::inc(&_times_locked);
	_holding_process = cur_proc;
}