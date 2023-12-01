/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "ProcessManager.h"

ProcessManager* ProcessManager::s_inst = nullptr;

ProcessManager::ProcessManager() {

}

ProcessManager& ProcessManager::inst() {
	if (!s_inst)
		s_inst = new ProcessManager();
	return *s_inst;
}

void ProcessManager::update() {
	m_processes = Sys::Process::get_all();
}

const std::map<pid_t, Sys::Process>& ProcessManager::processes() {
	return m_processes;
}
