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

#include "WaitBlocker.h"
#include "TaskManager.h"
#include "Thread.h"
#include "Process.h"

kstd::vector<kstd::Weak<WaitBlocker>> WaitBlocker::blockers;
kstd::vector<WaitBlocker::Notification> WaitBlocker::unhandled_notifications;
SpinLock WaitBlocker::lock;

kstd::Arc<WaitBlocker> WaitBlocker::make(kstd::Arc<Thread>& thread, pid_t wait_for, int options) {
	auto new_blocker = kstd::Arc<WaitBlocker>(new WaitBlocker(thread, wait_for, options));
	LOCK(WaitBlocker::lock);
	// Make sure there's no unhandled notification we couldn't handle
	for(size_t i = 0; i < unhandled_notifications.size(); i++) {
		auto& notif = unhandled_notifications[i];
		if (new_blocker->notify(notif.process, notif.reason, notif.status)) {
			unhandled_notifications.erase(i);
			break;
		}
	}
	blockers.push_back(new_blocker);
	return new_blocker;
}

WaitBlocker::WaitBlocker(kstd::Arc<Thread>& thread, pid_t wait_for, int options):
	_ppid(thread->process()->pid()),
	_wait_pid(wait_for),
	_options(options)
{
	if(_wait_pid < -1) {
		//Any child with pgid |pid|
		_wait_pgid = -_wait_pid;
	} else if(_wait_pid == 0) {
		//Any child in same group
		_wait_pgid = thread->process()->pgid();
	} else {
		//Any child / child with PID
		_wait_pgid = -1;
	}
}

bool WaitBlocker::is_ready() {
	return _ready;
}

Process* WaitBlocker::waited_process() {
	return _waited_process;
}

pid_t WaitBlocker::error() {
	return _err;
}

int WaitBlocker::status() {
	return _status;
}

void WaitBlocker::notify_all(Process* proc, WaitBlocker::Reason reason, int status) {
	if(proc->pid() == -1)
		return;
	LOCK(WaitBlocker::lock);
	for(size_t i = 0; i < blockers.size(); i++) {
		auto blocker = blockers[i].lock();
		if (!blocker) {
			blockers.erase(i);
			i--;
			continue;
		}
		if(blocker->notify(proc, reason, status)) {
			blockers.erase(i);
			return;
		}
	}
	if (reason == Exited || reason == Signalled)
		unhandled_notifications.push_back({proc, reason, status});
}

bool WaitBlocker::notify(Process* proc, WaitBlocker::Reason reason, int status) {
	if (_ready)
		return true;
	if (proc->ppid() != _ppid)
		return false;
	if (_wait_pgid != -1 && proc->pgid() != _wait_pgid)
		return false;
	if (_wait_pid != -1 && proc->pid() != _wait_pid)
		return false;
	_ready = true;
	_waited_process = proc;
	switch (reason) {
		case Exited:
			_status = __WIFEXITED | (status & 0x0f);
			break;
		case Signalled:
			_status = __WIFSIGNALED | (status & 0x0f);
			break;
		case Stopped:
			_status = __WIFSTOPPED;
			break;
	}
	return true;
}
