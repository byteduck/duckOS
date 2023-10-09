/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../tasking/WaitBlocker.h"
#include "../api/wait.h"

int Process::sys_waitpid(pid_t pid, UserspacePointer<int> status, int flags) {
	//TODO: Flags
	auto blocker = WaitBlocker::make(TaskManager::current_thread(), pid, flags);
	TaskManager::current_thread()->block(*blocker);
	if(blocker->was_interrupted())
		return -EINTR;
	if(blocker->error())
		return blocker->error();
	if(status)
		status.set(blocker->status());
	ASSERT(blocker->waited_process());
	pid_t ret = blocker->waited_process()->pid();
	if(WIFEXITED(blocker->status()) || WIFSIGNALED(blocker->status()))
		blocker->waited_process()->reap();
	return ret;
}
