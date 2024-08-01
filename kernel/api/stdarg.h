/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "cdefs.h"

__DECL_BEGIN

typedef __builtin_va_list va_list;

#define va_start(ap, pN) __builtin_va_start(ap, pN)

#define va_end(ap) __builtin_va_end(ap)

#define va_arg(ap, t) __builtin_va_arg(ap, t)

__DECL_END