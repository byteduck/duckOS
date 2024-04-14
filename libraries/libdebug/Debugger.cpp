/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "Debugger.h"
#include <cxxabi.h>

using namespace Debug;
using Duck::ResultRet, Duck::Result, Duck::Ptr;

Duck::ResultRet<AddressInfo> Debugger::info_at(size_t addr) {
	auto obj = TRY(object_at(addr));

	if (addr < obj->memloc || addr > (obj->memloc + obj->memsz))
		return Result("Symbol outside of object memory");

	auto sym = obj->symbolicate(addr - obj->memloc);
	if (!sym.name)
		return Result("No such symbol exists");

	// Demangle symbol name
	int status = 0;
	auto* demangled_name = abi::__cxa_demangle(sym.name, nullptr, nullptr, &status);
	const AddressInfo ret {
		.symbol_name = status == 0 ? demangled_name : sym.name,
		.symbol_offset = sym.offset,
		.object = obj
	};
	if (status == 0)
		free(demangled_name);
	return ret;
}

Duck::ResultRet<std::vector<size_t>> Debugger::walk_stack_unsymbolicated() {
	auto regs = TRY(get_registers());
	std::vector<size_t> ret;
	ret.push_back(regs.eip);
	while (true) {
		auto retaddr_res = peek(regs.ebp + 4);
		if (retaddr_res.is_error() || !retaddr_res.value())
			break;
		ret.push_back(retaddr_res.value());
		auto ebp_res = peek(regs.ebp);
		if (ebp_res.is_error())
			break;
		regs.ebp = ebp_res.value();
	}
	return ret;
}

Duck::ResultRet<std::vector<AddressInfo>> Debugger::walk_stack() {
	auto regs = TRY(get_registers());
	std::vector<AddressInfo> ret;
	ret.push_back(TRY(info_at(regs.eip)));
	while (true) {
		auto retaddr_res = peek(regs.ebp + 4);
		if (retaddr_res.is_error() || !retaddr_res.value())
			break;
		auto info_res = info_at(retaddr_res.value());
		if (info_res.is_error())
			ret.push_back({"???", retaddr_res.value(), nullptr});
		else
			ret.push_back(info_res.value());
		auto ebp_res = peek(regs.ebp);
		if (ebp_res.is_error())
			break;
		regs.ebp = ebp_res.value();
	}
	return ret;
}
