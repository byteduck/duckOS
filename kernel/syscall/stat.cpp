/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/VFS.h"

int Process::sys_fstat(int file, UserspacePointer<struct stat> buf) {
	if(file < 0 || file >= (int) _file_descriptors.size() || !_file_descriptors[file])
		return -EBADF;
	buf.checked<void>(true, 0, 1, [&]() {
		_file_descriptors[file]->metadata().stat(buf.raw());
	});
	return 0;
}

int Process::sys_stat(UserspacePointer<char> file, UserspacePointer<struct stat> buf) {
	kstd::string path = file.str();
	auto inode_or_err = VFS::inst().resolve_path(path, _cwd, _user);
	if(inode_or_err.is_error())
		return inode_or_err.code();
	buf.checked<void>(true, 0, 1, [&]() {
		inode_or_err.value()->inode()->metadata().stat(buf.raw());
	});
	return 0;
}

int Process::sys_lstat(UserspacePointer<char> file, UserspacePointer<struct stat> buf) {
	kstd::string path = file.str();
	auto inode_or_err = VFS::inst().resolve_path(path, _cwd, _user, nullptr, O_INTERNAL_RETLINK);
	if(inode_or_err.is_error())
		return inode_or_err.code();
	buf.checked<void>(true, 0, 1, [&]() {
		inode_or_err.value()->inode()->metadata().stat(buf.raw());
	});
	return 0;
}