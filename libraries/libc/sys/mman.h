/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "cdefs.h"
#include <kernel/api/mmap.h>

__DECL_BEGIN
void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
void* mmap_named(void* addr, size_t length, int prot, int flags, int fd, off_t offset, const char* name);
int munmap(void* addr, size_t length);
int mprotect(void *addr, size_t len, int prot);
__DECL_END