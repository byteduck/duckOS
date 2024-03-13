/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "types.h"

__DECL_BEGIN

#define UNIX_PATH_MAX 100

struct sockaddr_un {
	uint16_t sun_family;
	char sun_path[UNIX_PATH_MAX];
};

__DECL_END