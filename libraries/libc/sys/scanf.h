/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#ifndef DUCKOS_LIBC_SCANF_H
#define DUCKOS_LIBC_SCANF_H

#include <sys/cdefs.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdbool.h>

__DECL_BEGIN

int common_scanf(char* buffer, const char* format, va_list arg);

__DECL_END

#endif