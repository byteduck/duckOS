/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "types.h"

#define SHM_READ 0x1u
#define SHM_WRITE 0x2u
#define SHM_SHARE 0x4u

__DECL_BEGIN

struct shm {
	void* ptr;
	size_t size;
	int id;
};

__DECL_END