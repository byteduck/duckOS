/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "Reaper.h"
#include "TaskManager.h"
#include "../filesystem/procfs/ProcFS.h"

void kreaper_entry() {
	Reaper reaper;
	reaper.start();
}

Reaper* Reaper::s_inst = nullptr;

Reaper::Reaper() {
	ASSERT(!s_inst);
	s_inst = this;
}

Reaper& Reaper::inst() {
	return *s_inst;
}

void Reaper::reap(const kstd::Arc<Thread>& thread) {
	m_lock.acquire();
	m_queue.push_back(thread);
	m_lock.release();
	m_blocker.set_ready(true);
}

void Reaper::start() {
	while(1) {
		{
			LOCK(m_lock);
			while(!m_queue.empty()) {
				auto thread = m_queue.pop_front();
				thread->reap();
				if(thread->process()->state() == Process::ZOMBIE && !thread->process()->ppid()) {
					TaskManager::remove_process(thread->process());
					delete thread->process();
				}
			}
		}
		m_blocker.set_ready(false);
		TaskManager::current_thread()->block(m_blocker);
	}
}
