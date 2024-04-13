/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "LiveDebugger.h"
#include <sys/ptrace.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <libduck/MappedBuffer.h>

using namespace Debug;
using Duck::Result, Duck::ResultRet;

Result LiveDebugger::attach(pid_t pid, tid_t tid) {
	if (ptrace(PTRACE_ATTACH, tid, nullptr, nullptr) < 0)
		return Result(errno);
	m_pid = pid;
	m_tid = tid;
	TRYRES(reload_process_info());
	return Result::SUCCESS;
}

ResultRet<uintptr_t> Debug::LiveDebugger::peek(size_t addr) {
	if (!m_pid || !m_tid)
		return Result("Not attached");
	uintptr_t data;
	if (ptrace(PTRACE_PEEK, m_tid, (void*) addr, &data) < 0)
		return Result(errno);
	return data;
}

Result LiveDebugger::poke(size_t addr, uintptr_t val) {
	if (!m_pid || !m_tid)
		return Result("Not attached");
	if (ptrace(PTRACE_POKE, m_tid, (void*) addr, (void*) val) < 0)
		return Result(errno);
	return Result::SUCCESS;
}

ResultRet<Sys::Process::MemoryRegion> LiveDebugger::region_at(size_t addr) {
	for (auto& region : m_memory_regions) {
		if (region.start + region.size > addr) {
			// Assume they're in order
			if (region.start > addr)
				return Result("No such memory region");
			return region;
		}
	}
	return Result("No such memory region");
}

ResultRet<Exec::Object*> LiveDebugger::object_at(size_t addr) {
	auto reg = TRY(region_at(addr));
	if (reg.type != Sys::Process::MemoryRegion::Inode)
		return Result("Region not an object");

	auto loadedobj = m_objects.find(reg.name);
	if (loadedobj != m_objects.end()) {
		auto obj = loadedobj->second.get();
		if (!obj)
			return Result("No object at region"); // Previously failed to load
		return obj;
	}

	auto do_load = [&] () -> ResultRet<Duck::Ptr<Exec::Object>> {
		auto objfile = TRY(Duck::File::open(reg.name, "r"));
		auto mappedfile = TRY(Duck::MappedBuffer::make_file(objfile, Duck::MappedBuffer::R, Duck::MappedBuffer::SharedFile));
		auto obj = std::make_shared<Exec::Object>();
		obj->fd = objfile.fd();
		obj->mapped_file = mappedfile->data<uint8_t>();
		obj->mapped_size = mappedfile->size();
		obj->memloc = reg.start;
		obj->fd = objfile.fd();
		obj->name = Duck::Path(reg.name).basename();
		TRYRES(obj->load_for_debugger());
		objfile.set_close_on_destroy(false);
		mappedfile->set_unmap_on_destroy(false);
		return obj;
	};

	auto load_res = do_load();
	if (load_res.is_error()) {
		m_objects[reg.name] = nullptr;
		return load_res.result();
	}
	m_objects[reg.name] = load_res.value();
	return load_res.value().get();
}

Duck::ResultRet<PTraceRegisters> LiveDebugger::get_registers() {
	if (!m_pid || !m_tid)
		return Result("Not attached");
	PTraceRegisters ret;
	if (ptrace(PTRACE_GETREGS, m_tid, nullptr, &ret) < 0)
		return Result(errno);
	return ret;
}

Result LiveDebugger::reload_process_info() {
	m_process = TRY(Sys::Process::get(m_pid));
	m_memory_regions = TRY(m_process.memory_regions());
	return Result::SUCCESS;
}
