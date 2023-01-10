/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../tasking/SleepBlocker.h"

int Process::sys_sleep(UserspacePointer<timespec> time, UserspacePointer<timespec> remainder) {
	auto blocker = SleepBlocker(Time(time.get()));
	TaskManager::current_thread()->block(blocker);
	remainder.set(blocker.time_left().to_timespec());
	return blocker.was_interrupted() ? -EINTR : SUCCESS;
}