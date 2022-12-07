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

class Thread;
class SpinLock: public Lock, public Blocker {
public:
	SpinLock();
	~SpinLock();
	bool locked() override;
	void acquire() override;
	void release() override;
	bool is_ready() override;
	bool is_lock() override { return true; }
	Thread* responsible_thread() override;

private:
	Atomic<Thread*, MemoryOrder::AcqRel> m_holding_thread = 0;
	int m_times_locked = 0;
};

