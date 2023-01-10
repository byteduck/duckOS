/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/VFS.h"

int Process::sys_chdir(UserspacePointer<char> path) {
	kstd::string strpath = path.str();
	auto inode_or_error = VFS::inst().resolve_path(strpath, _cwd, _user);
	if(inode_or_error.is_error())
		return inode_or_error.code();
	if(!inode_or_error.value()->inode()->metadata().is_directory())
		return -ENOTDIR;
	_cwd = inode_or_error.value();
	return SUCCESS;
}