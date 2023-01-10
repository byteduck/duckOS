/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"

int Process::sys_sigaction(int sig, UserspacePointer<sigaction> new_action_ptr, UserspacePointer<sigaction> old_action) {
	if(sig == SIGSTOP || sig == SIGKILL || sig < 1 || sig >= 32)
		return -EINVAL;
	{
		if(old_action) {
			auto old = old_action.get();
			memcpy(&old.sa_sigaction, &signal_actions[sig].action, sizeof(Signal::SigAction::action));
			memcpy(&old.sa_flags, &signal_actions[sig].flags, sizeof(Signal::SigAction::flags));
			old_action.set(old);
		}

		auto new_action = new_action_ptr.get();

		//We don't want this interrupted or else we'd have a problem if it's needed before it's done
		TaskManager::enter_critical();
		signal_actions[sig].action = new_action.sa_sigaction;
		signal_actions[sig].flags = new_action.sa_flags;
		TaskManager::leave_critical();
	}
	return 0;
}