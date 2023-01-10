/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/VFS.h"

int Process::sys_truncate(UserspacePointer<char> path, off_t length) {
	return VFS::inst().truncate(path.str(), length, _user, _cwd).code();
}

int Process::sys_ftruncate(int file, off_t length) {
	return -1;
}