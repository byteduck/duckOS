/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "types.h"

#define PROT_EXEC	0x1
#define PROT_READ	0x2
#define PROT_WRITE	0x4
#define PROT_NONE	0x0

#define MAP_SHARED	0x1
#define MAP_PRIVATE	0x0

#define MAP_ANONYMOUS	0x2
#define MAP_ANON		MAP_ANONYMOUS
#define MAP_EXECUTABLE	0x0
#define MAP_FILE		0x0
#define MAP_FIXED		0x4
#define MAP_GROWSDOWN	0x8

#define MAP_FAILED ((void*) -1)

__DECL_BEGIN

struct mmap_args {
	void* addr;
	size_t length;
	int prot;
	int flags;
	int fd;
	off_t offset;
	void** addr_p;
	const char* name;
};

struct shmcreate_args {
	void* addr;
	size_t size;
	struct shm *shm;
	const char* name;
};

__DECL_END