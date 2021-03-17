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

#include <kernel/kstd/defines.h>
#include "WaitBlocker.h"

WaitBlocker::WaitBlocker(Process* proc, pid_t wait_for) {
	_process = proc;
	_wait_pid = wait_for;
	if(_wait_pid < -1) {
		//Any child with pgid |pid|
		_wait_pgid = -_wait_pid;
	} else if(_wait_pid == 0) {
		//Any child in same group
		_wait_pgid = _process->pgid();
	} else {
		//Any child / child with PID
		_wait_pgid = -1;
	}
}

bool WaitBlocker::is_ready() {
	bool found_one = false;
	Process *current = TaskManager::current_process();
	do{
		if(current->ppid() == _process->pid()) {
			found_one = true;
			if((_wait_pgid == -1 || current->pgid() == _wait_pgid) && (_wait_pid == -1 || current->pid() == _wait_pid) && current->state == PROCESS_ZOMBIE) {
				_wait_pid = current->pid();
				_exit_status = current->exit_status();
				current->reap();
				return true;
			}
		}
		current = current->next;
	} while(current != TaskManager::current_process());

	if(!found_one) _err = -ECHILD;
	return !found_one;
}

pid_t WaitBlocker::waited_pid() {
	return _wait_pid;
}

pid_t WaitBlocker::error() {
	return _err;
}

pid_t WaitBlocker::exit_status() {
	return _exit_status;
}