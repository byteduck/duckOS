/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../time/Time.h"

int Process::sys_gettimeofday(UserspacePointer<timeval> t, UserspacePointer<void*> z) {
	t.set(Time::now().to_timeval());
	return 0;
}