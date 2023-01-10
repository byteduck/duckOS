/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../tasking/WaitBlocker.h"

int Process::sys_waitpid(pid_t pid, UserspacePointer<int> status, int flags) {
	//TODO: Flags
	WaitBlocker blocker(TaskManager::current_thread(), pid);
	TaskManager::current_thread()->block(blocker);
	if(blocker.was_interrupted())
		return -EINTR;
	if(blocker.error())
		return blocker.error();
	if(status)
		status.set(blocker.exit_status());
	if(blocker.waited_process())
		delete blocker.waited_process();
	return blocker.waited_pid();
}
