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
Mutex WaitBlocker::lock {"WaitBlocker"};

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
	_options(options),
	_thread(thread)
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
	if (_ready.load(MemoryOrder::Acquire)) {
		// Since we mark ourselves ready before all threads are stopped, wait until the process is *actually* stopped.
		if (_reason == Stopped)
			return _waited_process->state() == Process::STOPPED;
		return true;
	}
	return false;
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
			if (reason == Stopped) {
				// We want to also notify our tracer if the reason is that we stopped...
				blockers.erase(i);
				i--;
			} else {
				return;
			}
		}
	}
	// TODO: Will this just get full after a while?
	unhandled_notifications.push_back({proc, reason, status});
}

bool WaitBlocker::notify(Process* proc, WaitBlocker::Reason reason, int status) {
	if (_ready.load())
		return true;
	if (proc->ppid() != _ppid && !proc->is_traced_by(_thread->process()))
		return false;
	if (_wait_pgid != -1 && proc->pgid() != _wait_pgid)
		return false;
	if (_wait_pid != -1 && proc->pid() != _wait_pid)
		return false;
	_reason = reason;
	_waited_process = proc;
	switch (reason) {
		case Exited:
			_status = __WIFEXITED | (status & 0xff);
			break;
		case Signalled:
			_status = __WIFSIGNALED | (status & 0xff);
			break;
		case Stopped:
			_status = __WIFSTOPPED | (status & 0xff);
			break;
	}
	_ready.store(true, MemoryOrder::Release);
	return true;
}
