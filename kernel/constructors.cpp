/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "constructors.h"
#include "kstd/kstdio.h"

typedef void (*constructor_func)();
extern constructor_func start_ctors[];
extern constructor_func end_ctors[];

//This method should be called with global constructors, so we'll assert that did_constructors == true after we do that
bool did_constructors = false;
__attribute__((constructor)) void constructor_test() {
	did_constructors = true;
}

void call_global_constructors() {
	for (constructor_func* ctor = start_ctors; ctor < end_ctors; ctor++)
		(*ctor)();
	ASSERT(did_constructors);
}