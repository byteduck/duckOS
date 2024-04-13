/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Loader.h"
#include "dlfunc.h"

using Duck::Result, Duck::ResultRet;
using namespace Exec;

Duck::ResultRet<Object*> __dlopen(const char* filename, int flags) {
	auto loader = Loader::main();
	if (!loader)
		return Result(EINVAL);

	auto obj = loader->open_library(filename);
	if (!obj)
		return Result(ENOENT);
	obj->load(*loader, filename);
	obj->relocate(*loader);
	obj->mprotect_sections();
	return obj;
}

Result __dlclose(Exec::Object* obj) {
	// TODO
	return Result::SUCCESS;
}

ResultRet<uintptr_t> __dlsym(Exec::Object* obj, const char* name) {
	if (obj == nullptr) { // RTLD_DEFAULT
		auto loader = Loader::main();
		if (!loader)
			return Result(EINVAL);
		auto sym = loader->get_symbol(name);
		if (!sym)
			sym = loader->get_global_symbol(name);
		return sym;
	}
	return obj->get_dynamic_symbol(name);
}