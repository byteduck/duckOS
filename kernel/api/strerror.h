/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include "errno.h"
#include "cdefs.h"

__DECL_BEGIN
char* strerror(int errnum);
__DECL_END