/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"

int Process::sys_threadcreate(void* (*entry_func)(void* (*)(void*), void*), void* (*thread_func)(void*), void* arg) {
	auto thread = kstd::make_shared<Thread>(_self_ptr, TaskManager::get_new_pid(), entry_func, thread_func, arg);
	recalculate_pmem_total();
	insert_thread(thread);
	{
		CRITICAL_LOCK(TaskManager::g_tasking_lock);
		TaskManager::queue_thread(thread);
	}
	return thread->tid();
}

int Process::sys_gettid() {
	return TaskManager::current_thread()->tid();
}

int Process::sys_threadjoin(tid_t tid, UserspacePointer<void*> retp) {
	auto cur_thread = TaskManager::current_thread();
	auto thread = get_thread(tid);
	if(tid > _threads.size() || !thread)
		return -ESRCH;
	Result result = cur_thread->join(cur_thread, thread, retp);
	if(result.is_success()) {
		ASSERT(thread->state() == Thread::DEAD);
		thread.reset();
	}
	return result.code();
}

int Process::sys_threadexit(void* return_value) {
	TaskManager::current_thread()->exit(return_value);
	ASSERT(false);
	return -1;
}