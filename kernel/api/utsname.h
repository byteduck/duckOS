/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "types.h"

__DECL_BEGIN

struct utsname {
	char sysname[16];
	char nodename[64];
	char release[16];
	char version[32];
	char machine[16];
};

__DECL_END