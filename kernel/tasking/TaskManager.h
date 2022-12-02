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

class Process;
class Thread;
class SpinLock;
struct TSS;

namespace TaskManager {
	extern TSS tss;
	extern SpinLock lock;

	void init();
	bool& enabled();
	bool is_idle();
	bool is_preempting();
	void reparent_orphans(Process* proc);

	kstd::vector<Process*>* process_list();
	int add_process(Process* proc);
	void queue_thread(const kstd::Arc<Thread>& thread);
	kstd::Arc<Thread>& current_thread();
	Process* current_process();
	ResultRet<Process*> process_for_pid(pid_t pid);
	ResultRet<Process*> process_for_pgid(pid_t pgid, pid_t exclude = -1);
	ResultRet<Process*> process_for_ppid(pid_t ppid, pid_t exclude = -1);
	ResultRet<Process*> process_for_sid(pid_t sid, pid_t exclude = -1);

	void kill_pgid(pid_t pgid, int sig);

	bool yield();
	bool yield_if_not_preempting();
	bool yield_if_idle();
	void do_yield_async();

	void notify_current(uint32_t sig);

	pid_t get_new_pid();
	kstd::Arc<Thread> next_thread();

	extern "C" void preempt();
	extern "C" void __attribute((cdecl)) preempt_init_asm(unsigned int new_esp);
	extern "C" void __attribute((cdecl)) preempt_asm(unsigned int *old_esp, unsigned int *new_esp, uint32_t new_cr3);
	extern "C" void proc_first_preempt();

	class Disabler {
	public:
		Disabler();
		~Disabler();

	private:
		bool _enabled;
	};
};

