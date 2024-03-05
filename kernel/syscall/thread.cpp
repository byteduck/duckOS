/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"

int Process::sys_threadcreate(void* (*entry_func)(void* (*)(void*), void*), void* (*thread_func)(void*), void* arg) {
	auto thread = kstd::make_shared<Thread>(_self_ptr, TaskManager::get_new_pid(), entry_func, thread_func, arg);
	insert_thread(thread);
	TaskManager::queue_thread(thread);
	return thread->tid();
}

int Process::sys_gettid() {
	return TaskManager::current_thread()->tid();
}

int Process::sys_threadjoin(tid_t tid, UserspacePointer<void*> retp) {
	auto cur_thread = TaskManager::current_thread();
	auto thread = get_thread(tid);
	if(!thread) {
		// See if the thread died already.
		LOCK(_thread_lock);
		auto return_val = _thread_return_values.find_node(tid);
		if(!return_val)
			return -ESRCH;
		if(retp)
			retp.set(return_val->data.second);
		return SUCCESS;
	}
	Result result = cur_thread->join(cur_thread, thread, retp);
	if(result.is_success()) {
		ASSERT(thread->state() == Thread::DEAD || thread->_waiting_to_die);
		thread.reset();
	}
	return result.code();
}

int Process::sys_threadexit(void* return_value) {
	TaskManager::current_thread()->exit(return_value);
	TaskManager::current_thread()->leave_critical(); /* Leave critical early, since we enter it in the syscall handler */
	ASSERT(false);
	return -1;
}