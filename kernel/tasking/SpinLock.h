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

#ifndef DUCKOS_SPINLOCK_H
#define DUCKOS_SPINLOCK_H

#include "Lock.h"
#include "BooleanBlocker.h"
#include <common/shared_ptr.hpp>

class Process;
class SpinLock: public Lock {
public:
	SpinLock();
	~SpinLock();
	bool locked() override;
	void acquire() override;
	void release() override;
private:
	BooleanBlocker _blocker;
	volatile int _locked = 0;
	volatile int _times_locked = 0;
	volatile Process* _holding_process = nullptr;
};


#endif //DUCKOS_SPINLOCK_H
