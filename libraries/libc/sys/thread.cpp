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

#include <map>
#include <atomic>
#include "thread.h"
#include "syscall.h"

void thread_entry(void* (*entry_func)(void*), void* arg) {
	void* ret = entry_func(arg);
	thread_exit(ret);
}

tid_t thread_create(void* (*entry_func)(void*), void* arg) {
	return syscall4(SYS_THREADCREATE, (int)thread_entry, (int)entry_func, (int)arg);
}

void thread_exit(void* retval) {
	syscall2(SYS_THREADEXIT, (int) retval);
}

int thread_join(tid_t thread, void** retval) {
	return syscall3(SYS_THREADJOIN, thread, (int) retval);
}

tid_t gettid() {
	return syscall_noerr(SYS_GETTID);
}