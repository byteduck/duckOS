/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/VFS.h"

int Process::sys_chmod(UserspacePointer<char> file, mode_t mode) {
	return VFS::inst().chmod(file.str(), mode, _user, _cwd).code();
}

int Process::sys_fchmod(int fd, mode_t mode) {
	return -1;
}

int Process::sys_chown(UserspacePointer<char> file, uid_t uid, gid_t gid) {
	return VFS::inst().chown(file.str(), uid, gid, _user, _cwd).code();
}

int Process::sys_fchown(int fd, uid_t uid, gid_t gid) {
	return -1;
}

int Process::sys_lchown(UserspacePointer<char> file, uid_t uid, gid_t gid) {
	return VFS::inst().chown(file.str(), uid, gid, _user, _cwd, O_NOFOLLOW).code();
}