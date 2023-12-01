/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once
#include <sys/ptrace.h>
#include <libduck/Result.h>

namespace Debug {
	class Debugger {
	public:
		Debugger() = default;

		Duck::Result attach(pid_t pid);
		Duck::Result detach();

	private:
		pid_t m_attached_pid = 0;
	};
}