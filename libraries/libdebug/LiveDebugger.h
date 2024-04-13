/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "Debugger.h"

namespace Debug {
	class LiveDebugger: public Debugger {
	public:
		Duck::Result attach(pid_t pid, tid_t tid);

		// Debugger
		Duck::ResultRet<uintptr_t> peek(size_t addr) override;
		Duck::Result poke(size_t addr, uintptr_t val) override;
		Duck::ResultRet<Sys::Process::MemoryRegion> region_at(size_t addr) override;
		Duck::ResultRet<Exec::Object*> object_at(size_t addr) override;
		Duck::ResultRet<PTraceRegisters> get_registers() override;

	private:
		Duck::Result reload_process_info();

		pid_t m_pid = 0, m_tid = 0;
		std::vector<Sys::Process::MemoryRegion> m_memory_regions;
		std::map<std::string, Duck::Ptr<Exec::Object>> m_objects;
		Sys::Process m_process;
	};
}