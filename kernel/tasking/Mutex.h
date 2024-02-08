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

#pragma once

#include "Lock.h"
#include "BooleanBlocker.h"
#include <kernel/kstd/Arc.h>
#include "../kstd/unix_types.h"
#include "../Atomic.h"

#define CRITICAL_LOCK(lock) ScopedCriticalLocker __locker((lock));

class Thread;
class Mutex: public Lock {
public:
	enum class AcquireMode {
		EnterCritical, ///< The CPU will enter critical mode while acquiring the lock, and will not leave once acquired.
		Normal, ///< The CPU will acquire the lock without entering critical mode.
		Try ///< The CPU will try to acquire the lock, but will return if it is already locked.
	};

	explicit Mutex(const kstd::string& name);
	~Mutex();
	bool locked() override;
	void acquire() override;
	bool try_acquire();
	void acquire_and_enter_critical();
	void release() override;
	bool held_by_current_thread();
	[[nodiscard]] tid_t holding_thread() const { return m_holding_thread.load(MemoryOrder::SeqCst); }
	[[nodiscard]] int times_locked() const { return m_times_locked.load(); }

private:
	template<AcquireMode mode>
	inline bool acquire_with_mode();

	Atomic<tid_t, MemoryOrder::SeqCst> m_holding_thread = -1;
	Atomic<int, MemoryOrder::SeqCst> m_times_locked = 0;
};

class ScopedCriticalLocker {
public:
	ScopedCriticalLocker(Mutex& lock);
	~ScopedCriticalLocker();
	void release();

private:
	Mutex& m_lock;
	bool m_released = false;
};
