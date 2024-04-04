/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "dlfcn.h"
#include <libexec/dlfunc.h>

#pragma weak __dlopen
#pragma weak __dlclose
#pragma weak __dlsym

void* dlopen(const char* name, int flags) {
	auto res = __dlopen(name, flags);
	if (res.is_error()) {
		errno = res.code();
		return nullptr;
	}
	return res.value();
}

int dlclose(void* object) {
	auto res = __dlclose((Exec::Object*) object);
	if (res.is_error()) {
		errno = res.code();
		return -1;
	}
	return 0;
}

void* dlsym(void* object, const char* name) {
	auto res = __dlsym((Exec::Object*) object, name);
	if (res.is_error()) {
		errno = res.code();
		return nullptr;
	}
	return (void*) res.value();
}

char* dlerror() {
	// TODO: Only dl errors
	return strerror(errno);
}