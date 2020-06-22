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

#define PROCESS_ALIVE 0
#define PROCESS_ZOMBIE 1
#define PROCESS_DEAD 2

#define SIGTERM 15
#define SIGILL 4
#define SIGSEGV 11

class Process;

namespace TaskManager {
	extern TSS tss;
	void init();
	bool& enabled();
	void print_tasks();
	uint32_t add_process(Process *p);
	Process *current_process();
	void preempt_now();
	pid_t get_new_pid();
	Process *process_for_pid(pid_t pid);
	extern "C" void preempt();
	extern "C" void preempt_init_asm(unsigned int new_esp);
	extern "C" void preempt_asm(unsigned int *old_esp, unsigned int *new_esp, uint32_t new_cr3);
	extern "C" void iret(void);
	void notify(uint32_t sig);
	void kill(Process *p);
};

#endif
