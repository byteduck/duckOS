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
	return m_holding_thread.load();
}

void SpinLock::release() {
	if(!TaskManager::enabled())
		return;

	// Decrease counter. If the counter is zero, release the lock
	m_times_locked--;
	if(!m_times_locked)
		m_holding_thread.store(nullptr);
}

void SpinLock::acquire() {
	auto cur_thread = TaskManager::current_thread();
	if(!TaskManager::enabled() || !cur_thread) return; //Tasking isn't initialized yet

	//Loop while the lock is held
	while(true) {
		// Try locking if no thread is holding
		Thread* expected = nullptr;
		if(m_holding_thread.compare_exchange_strong(expected, cur_thread.get()))
			break;

		// Try locking if current thread is holding
		expected = cur_thread.get();
		if(m_holding_thread.compare_exchange_strong(expected, cur_thread.get()))
			break;

		// Block until unlocked
		cur_thread->block(*this);
	}

	// We've got the lock!
	m_times_locked++;
}

bool SpinLock::is_ready() {
	return m_holding_thread.load() == 0;
}

Thread* SpinLock::responsible_thread() {
	return m_holding_thread.load();
}