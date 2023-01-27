/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <kernel/api/utsname.h>

__DECL_BEGIN

int uname(struct utsname* buf);

__DECL_END