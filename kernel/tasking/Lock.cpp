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

#include "Lock.h"
#include "Mutex.h"
#include "TaskManager.h"
#include <kernel/kstd/map.hpp>

#ifdef DEBUG
// Hmm, I need a simple set type, don't I.
Mutex g_lock_registry_lock {"LockRegistry"};
kstd::map<Lock*, int> g_lock_registry;
#endif

Lock::Lock(const kstd::string& name): m_name(name) {
#ifdef DEBUG
	if (__builtin_expect(this != &g_lock_registry_lock, true)) {
		if (__builtin_expect(TaskManager::enabled(), true)) {
			LOCK(g_lock_registry_lock);
			g_lock_registry[this] = 1;
		} else {
			g_lock_registry[this] = 1;
		}
	}
#endif
}

#ifdef DEBUG
Lock::~Lock() {
	LOCK(g_lock_registry_lock);
	g_lock_registry.erase(this);
}
#else
Lock::~Lock() = default;
#endif

ScopedLocker::ScopedLocker(Lock& lock): _lock(lock) {
	_lock.acquire();
}

ScopedLocker::~ScopedLocker() {
	_lock.release();
}
