/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../api/utsname.h"
#include <duckos_version.h>

int Process::sys_uname(UserspacePointer<struct utsname> buf) {
	utsname ret {
			"duckOS",
			"default",
			"",
			"",
			"i386"
	};

	strcpy(ret.release, DUCKOS_VERSION_STRING);
	strcpy(ret.version, DUCKOS_REVISION);

	buf.set(ret);
	return SUCCESS;
}