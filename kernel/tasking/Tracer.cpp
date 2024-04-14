/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Tracer.h"
#include "Thread.h"
#include "Process.h"

Tracer::Tracer(Process* tracer, kstd::Arc<Thread> tracee):
	m_tracer(tracer), m_tracee(kstd::move(tracee)) {}

void Tracer::set_signal(int signal) {
	ASSERT(signal > 0 && signal < 32);
	m_signals |= (1 << signal);
}

bool Tracer::has_signal(int signal) {
	ASSERT(signal > 0 && signal < 32);
	return m_signals & (1 << signal);
}

void Tracer::clear_signal(int signal) {
	ASSERT(signal > 0 && signal < 32);
	m_signals &= ~(1 << signal);
}


