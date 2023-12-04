/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <vector>
#include <libsys/Process.h>

class ProcessManager {
public:
	static ProcessManager& inst();

	void update();
	const std::map<pid_t, Sys::Process>& processes();

private:
	ProcessManager();
	static ProcessManager* s_inst;

	std::map<pid_t, Sys::Process> m_processes;
};
