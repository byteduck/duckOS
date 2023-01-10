/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/VFS.h"

int Process::sys_rmdir(UserspacePointer<char> name) {
	auto ret = VFS::inst().rmdir(name.str(), _user, _cwd);
	if(ret.is_error())
		return ret.code();
	return 0;
}

int Process::sys_mkdir(UserspacePointer<char> path, mode_t mode) {
	mode &= 04777; //We just want the permission bits
	auto ret = VFS::inst().mkdir(path.str(), mode, _user, _cwd);
	if(ret.is_error())
		return ret.code();
	return 0;
}

int Process::sys_mkdirat(int file, UserspacePointer<char> path, mode_t mode) {
	return -1;
}