/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../tasking/TaskManager.h"

pid_t Process::sys_fork(ThreadRegisters& regs) {
	auto* new_proc = new Process(this, regs);
	// If the process execs before sys_fork finishes, pid would be -1 so we save it here
	auto pid = new_proc->pid();
	TaskManager::add_process(new_proc->_self_ptr);
	return pid;
}
