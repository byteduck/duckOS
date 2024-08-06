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

#pragma once

#include <kernel/kstd/vector.hpp>
#include <kernel/kstd/Arc.h>
#include <kernel/Result.hpp>
#include <kernel/kstd/unix_types.h>
#include "Thread.h"
#include "Process.h"
#include "../arch/tasking.h"

class Process;
class Thread;
class Mutex;
struct TSS;

namespace TaskManager {
	extern TSS tss;

	/** This lock is acquired while preempting to ensure that thread queues are in a valid state. This lock MUST be
	 *  held prior to calling queue_thread or messing with the thread queue. You should use a ScopedCriticalLocker or
	 *  the CRITICAL_LOCK macro to acquire g_tasking_lock and enter a critical state which will automatically be
	 *  released after leaving the scope.
     */
	extern Mutex g_tasking_lock;

	/** This lock is acquired while editing the process list. **/
	extern Mutex g_process_lock;

	/** This is the thread at the beginning of the thread queue. The thread queue is updated on calls to `queue_thread`
	 *  and in Thread::reap (which ensures that the reaped thread is removed from the queue.
	 */
	extern Thread* g_next_thread;

	void init();
	void idle_task();
	bool enabled();
	bool is_idle();
	bool is_preempting();
	void reparent_orphans(Process* proc);

	kstd::vector<Process*>* process_list();
	int add_process(Process* proc);
	void remove_process(Process* proc);
	void queue_thread(const kstd::Arc<Thread>& thread);
	kstd::Arc<Thread>& current_thread();
	Process* current_process();
	ResultRet<kstd::Arc<Thread>> thread_for_tid(tid_t tid);
	ResultRet<Process*> process_for_pid(pid_t pid);
	ResultRet<Process*> process_for_pgid(pid_t pgid, pid_t exclude = -1);
	ResultRet<Process*> process_for_ppid(pid_t ppid, pid_t exclude = -1);
	ResultRet<Process*> process_for_sid(pid_t sid, pid_t exclude = -1);

	void kill_pgid(pid_t pgid, int sig);

	bool yield();
	bool yield_if_not_preempting();
	bool yield_if_idle();
	void do_yield_async();
	void tick();

	void enter_critical();
	extern "C" void leave_critical();
	bool in_critical();

	class ScopedCritical {
	public:
		ScopedCritical() {
			TaskManager::enter_critical();
		}

		~ScopedCritical() {
			if(!m_done)
				TaskManager::leave_critical();
		}

		void exit() {
			m_done = true;
			TaskManager::leave_critical();
		}

	private:
		bool m_done = false;
	};

	pid_t get_new_pid();
	kstd::Arc<Thread> pick_next_thread();


	extern "C" void preempt();
	extern "C" void preempt_finish();
	extern "C" void proc_first_preempt();
};

