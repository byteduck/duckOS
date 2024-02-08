/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "BooleanBlocker.h"
#include "kernel/kstd/queue.hpp"
#include "Mutex.h"

void kreaper_entry();

class Reaper {
public:
	Reaper();
	static Reaper& inst();

	void reap(kstd::Weak<Thread> thread);

protected:
	friend void kreaper_entry();
	void start();

private:
	Mutex m_lock {"Reaper"};
	BooleanBlocker m_blocker;
	kstd::queue <kstd::Arc<Thread>> m_queue;
	static Reaper* s_inst;
};
