/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/VFS.h"

int Process::sys_access(UserspacePointer<char> pathname, int mode) {
	return VFS::inst().access(pathname.str(), mode, _user, _cwd).code();
}