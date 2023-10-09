/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"

int Process::sys_kill(pid_t pid, int sig) {
	//TODO: Permission check
	if(sig == 0)
		return 0;
	if(sig < 0 || sig >= NSIG)
		return -EINVAL;
	if(pid == _pid)
		kill(sig);
	else if(pid == 0) {
		//Kill all processes with _pgid == this->_pgid
		auto* procs = TaskManager::process_list();
		for(int i = 0; i < procs->size(); i++) {
			auto c_proc = procs->at(i);
			if(c_proc != this && (_user.uid == 0 || c_proc->_user.uid == _user.uid) && c_proc->_pgid == _pgid && c_proc->_pid > 1)
				c_proc->kill(sig);
		}
	} else if(pid == -1) {
		//kill all processes for which we have permission to kill except init
		auto* procs = TaskManager::process_list();
		for(int i = 0; i < procs->size(); i++) {
			auto c_proc = procs->at(i);
			if(c_proc != this && (_user.uid == 0 || c_proc->_user.uid == _user.uid) && c_proc->_pid > 1)
				c_proc->kill(sig);
		}
	} else if(pid < -1) {
		//Kill all processes with _pgid == -pid
		auto* procs = TaskManager::process_list();
		for(int i = 0; i < procs->size(); i++) {
			auto c_proc = procs->at(i);
			if((_user.uid == 0 || c_proc->_user.uid == _user.uid) && c_proc->_pgid == -pid && c_proc->_pid > 1)
				c_proc->kill(sig);
		}
		kill(sig);
	} else {
		//Kill process with _pid == pid
		auto proc = TaskManager::process_for_pid(pid);
		if(proc.is_error())
			return -ESRCH;
		if((_user.uid != 0 && proc.value()->_user.uid != _user.uid) || proc.value()->_pid <= 1)
			return -EPERM;
		proc.value()->kill(sig);
	}
	return 0;
}