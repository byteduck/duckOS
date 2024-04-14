/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "../kstd/Arc.h"


class Process;
class Thread;
class Tracer {
public:
	Tracer(Process* tracer, kstd::Arc<Thread> tracee);

	void set_signal(int signal);
	bool has_signal(int signal);
	void clear_signal(int signal);

	[[nodiscard]] Process* tracer_process() const { return m_tracer; }
	[[nodiscard]] const kstd::Arc<Thread>& tracee_thread() const { return m_tracee; }

private:
	Process* m_tracer;
	kstd::Arc<Thread> m_tracee;
	uint32_t m_signals = 0;
};
