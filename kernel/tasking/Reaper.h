/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "BooleanBlocker.h"
#include "kernel/kstd/queue.hpp"
#include "SpinLock.h"

void kreaper_entry();

class Reaper {
public:
	Reaper();
	static Reaper& inst();

	void reap(Process* process);

protected:
	friend void kreaper_entry();
	void start();

private:
	SpinLock m_lock;
	BooleanBlocker m_blocker;
	kstd::queue <Process*> m_queue;
	static Reaper* s_inst;
};
