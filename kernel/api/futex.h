/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include "types.h"

#define FUTEX_INIT    1
#define FUTEX_DESTROY 2
#define FUTEX_WAIT    3

__DECL_BEGIN

typedef int futex_t;

__DECL_END