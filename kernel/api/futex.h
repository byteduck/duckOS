/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include "types.h"

#define FUTEX_WAIT    1
#define FUTEX_REGFD   2

__DECL_BEGIN

typedef int futex_t;

__DECL_END