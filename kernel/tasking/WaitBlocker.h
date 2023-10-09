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

#pragma once

#include "Blocker.h"
#include <kernel/kstd/unix_types.h>
#include <kernel/kstd/Arc.h>
#include <kernel/api/wait.h>
#include <kernel/kstd/vector.hpp>
#include <kernel/tasking/SpinLock.h>

class Thread;
class WaitBlocker: public Blocker {
public:
	enum Reason {
		Exited, Signalled, Stopped
	};

	static kstd::Arc<WaitBlocker> make(kstd::Arc<Thread>& thread, pid_t wait_for, int options);
	static void notify_all(Process* proc, Reason reason, int status);

	bool is_ready() override;

	Process* waited_process();
	pid_t error();
	int status();

private:
	struct Notification {
		Process* process;
		Reason reason;
		int status;
	};

	WaitBlocker(kstd::Arc<Thread>& thread, pid_t wait_for, int options);

	bool notify(Process* proc, Reason reason, int status);

	static kstd::vector<kstd::Weak<WaitBlocker>> blockers;
	static kstd::vector<Notification> unhandled_notifications;
	static SpinLock lock;

	bool _ready = false;
	int _err = 0;
	int _status;
	Process* _waited_process = nullptr;
	pid_t _wait_pid;
	pid_t _wait_pgid;
	int _options;
	pid_t _ppid;
};

