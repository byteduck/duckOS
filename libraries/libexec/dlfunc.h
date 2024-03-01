/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include <libduck/Result.h>
#include "Object.h"

extern "C" {
	Duck::ResultRet<Exec::Object*> __dlopen(const char* name, int flags);
	Duck::Result __dlclose(Exec::Object* obj);
	Duck::ResultRet<uintptr_t> __dlsym(Exec::Object* obj, const char* name);
}