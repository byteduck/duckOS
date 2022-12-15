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

void Reaper::reap(Process* process) {
	ASSERT(process->state() == Process::DEAD);
	LOCK(m_lock);
	m_queue.push_back(process);
	m_blocker.set_ready(true);
}

void Reaper::start() {
	while(1) {
		m_lock.acquire();

		{
			TaskManager::ScopedCritical critical;
			while(!m_queue.empty()) {
				auto process = m_queue.pop_front();
				process->free_resources();
				ProcFS::inst().proc_remove(process);
				TaskManager::remove_process(process);
				delete process;
			}
		}

		m_lock.release();
		m_blocker.set_ready(false);
		TaskManager::current_thread()->block(m_blocker);
	}
}
