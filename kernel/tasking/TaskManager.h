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

#ifndef TASKING_H
#define TASKING_H

#include <kernel/kstddef.h>
#include "Process.h"
#include "TSS.h"
#include "SpinLock.h"

class Process;
class SpinLock;
namespace TaskManager {
	extern TSS tss;
	extern SpinLock lock;

	void init();
	bool& enabled();
	bool is_idle();
	void print_tasks();

	uint32_t add_process(Process *p);
	Process *current_process();
	Process *process_for_pid(pid_t pid);
	Process *process_for_pgid(pid_t pgid, pid_t exclude = -1);
	Process *process_for_ppid(pid_t ppid, pid_t exclude = -1);
	Process *process_for_sid(pid_t sid, pid_t exclude = -1);

	bool yield();
	bool yield_if_idle();
	void do_yield_async();

	void notify_current(uint32_t sig);

	pid_t get_new_pid();
	Process* next_process();

	extern "C" void preempt();
	extern "C" void preempt_now_asm();
	extern "C" void __attribute((cdecl)) preempt_init_asm(unsigned int new_esp);
	extern "C" void __attribute((cdecl)) preempt_asm(unsigned int *old_esp, unsigned int *new_esp, uint32_t new_cr3);
	extern "C" void proc_first_preempt();
};

#endif
