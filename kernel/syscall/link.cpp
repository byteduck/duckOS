/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/VFS.h"

int Process::sys_link(UserspacePointer<char> oldpath, UserspacePointer<char> newpath) {
	return VFS::inst().link(oldpath.str(), newpath.str(), _user, _cwd).code();
}

int Process::sys_unlink(UserspacePointer<char> name) {
	auto ret = VFS::inst().unlink(name.str(), _user, _cwd);
	if(ret.is_error())
		return ret.code();
	return 0;
}

int Process::sys_symlink(UserspacePointer<char> file, UserspacePointer<char> linkname) {
	return VFS::inst().symlink(file.str(), linkname.str(), _user, _cwd).code();
}

int Process::sys_symlinkat(UserspacePointer<char> file, int dirfd, UserspacePointer<char> linkname) {
	return -1;
}

int Process::sys_readlink(UserspacePointer<char> file, UserspacePointer<char> buf, size_t bufsize) {
	if(bufsize < 0)
		return -EINVAL;

	ssize_t ret;
	auto ret_perhaps = VFS::inst().readlink(file.str(), _user, _cwd, ret);
	if(ret_perhaps.is_error())
		return ret_perhaps.code();

	auto& link_value = ret_perhaps.value();
	buf.write(link_value.c_str(), min(link_value.length(), bufsize - 1));
	buf.set(min(bufsize - 1, link_value.length()), '\0');

	return SUCCESS;
}

int Process::sys_readlinkat(UserspacePointer<readlinkat_args> args) {
	return -1;
}