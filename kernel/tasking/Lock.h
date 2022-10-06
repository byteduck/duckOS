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

class Lock {
public:
	virtual bool locked() = 0;
	virtual void acquire() = 0;
	virtual void release() = 0;

	template<typename R, typename F>
	R synced(F&& lambda) {
		acquire();
		R ret = lambda();
		release();
		return ret;
	}

	template<typename F>
	void synced(F&& lambda) {
		acquire();
		lambda();
		release();
	}
};

class ScopedLocker {
public:
	explicit ScopedLocker(Lock& lock);
	~ScopedLocker();
private:
	Lock& _lock;
};

#define LOCK(lock) ScopedLocker __locker((lock))
#define LOCK_N(lock, name) ScopedLocker name((lock))

