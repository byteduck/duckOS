/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once
#include <sys/ptrace.h>
#include <libduck/Result.h>
#include <libsys/Process.h>
#include <libexec/Object.h>
#include "Info.h"
#include <sys/registers.h>

namespace Debug {

	/// Generic interface for debugging - could either be a live process or core dump.
	class Debugger {
	public:
		Debugger() = default;

		virtual Duck::ResultRet<uintptr_t> peek(size_t addr) = 0;
		virtual Duck::Result poke(size_t addr, uintptr_t val) = 0;
		virtual Duck::ResultRet<Sys::Process::MemoryRegion> region_at(size_t addr) = 0;
		virtual Duck::ResultRet<Exec::Object*> object_at(size_t addr) = 0;
		virtual Duck::ResultRet<PTraceRegisters> get_registers() = 0;

		Duck::ResultRet<AddressInfo> info_at(size_t addr);
		Duck::ResultRet<std::vector<AddressInfo>> walk_stack();
		Duck::ResultRet<std::vector<size_t>> walk_stack_unsymbolicated();
	};
}