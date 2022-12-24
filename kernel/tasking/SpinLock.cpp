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

SpinLock::SpinLock() = default;

SpinLock::~SpinLock() = default;

bool SpinLock::locked() {
	return m_holding_thread.load(MemoryOrder::SeqCst);
}

void SpinLock::release() {
	if(!TaskManager::enabled())
		return;

	// Decrease counter. If the counter is zero, release the lock
	m_times_locked--;
	if(!m_times_locked)
		m_holding_thread.store(nullptr, MemoryOrder::SeqCst);
}

void SpinLock::acquire() {
	acquire_with_mode(AcquireMode::Normal);
	ASSERT(!TaskManager::in_critical());
}

bool SpinLock::try_acquire() {
	return acquire_with_mode(AcquireMode::Try);
}

void SpinLock::acquire_and_enter_critical() {
	acquire_with_mode(AcquireMode::EnterCritical);
}

inline bool SpinLock::acquire_with_mode(AcquireMode mode) {
	auto cur_thread = TaskManager::current_thread();
	if(!TaskManager::enabled() || !cur_thread)
		return true; //Tasking isn't initialized yet

	//Loop while the lock is held
	while(true) {
		if(mode == AcquireMode::EnterCritical)
			TaskManager::enter_critical();

		// Try locking if no thread is holding
		Thread* expected = nullptr;
		if(m_holding_thread.compare_exchange_strong(expected, cur_thread.get()))
			break;

		// Try locking if current thread is holding
		expected = cur_thread.get();
		if(m_holding_thread.compare_exchange_strong(expected, cur_thread.get()))
			break;

		if(mode == AcquireMode::EnterCritical)
			TaskManager::leave_critical();
		else if(mode == AcquireMode::Try)
			return false;

		ASSERT(!TaskManager::in_critical());
		TaskManager::yield();
	}

	// We've got the lock!
	m_times_locked++;
	return true;
}

bool SpinLock::held_by_current_thread() {
	return TaskManager::current_thread().get() == m_holding_thread.load(MemoryOrder::SeqCst);
}