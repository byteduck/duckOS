/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#ifndef __DUCKOS_LIBC_DLFCN_H
#define __DUCKOS_LIBC_DLFCN_H

#include <sys/cdefs.h>

__DECL_BEGIN

#define RTLD_DEFAULT 0
#define RTLD_LAZY 2
#define RTLD_NOW 4

void* dlopen(const char*, int);
int dlclose(void*);
void* dlsym(void*, const char*);

__DECL_END

#endif