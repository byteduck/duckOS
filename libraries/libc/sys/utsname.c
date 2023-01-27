/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "utsname.h"
#include "syscall.h"

int uname(struct utsname* buf) {
	return syscall2(SYS_UNAME, (int) buf);
}