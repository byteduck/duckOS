/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../filesystem/FileDescriptor.h"
#include <kernel/filesystem/VFS.h>

ssize_t Process::sys_read(int fd, UserspacePointer<uint8_t> buf, size_t count) {
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd])
		return -EBADF;
	return  _file_descriptors[fd]->read(buf, count);
}

int Process::sys_readdir(int file, UserspacePointer<char> buf, size_t len) {
	if(file < 0 || file >= (int) _file_descriptors.size() || !_file_descriptors[file])
		return -EBADF;
	return _file_descriptors[file]->read_dir_entries(buf, len);
}

ssize_t Process::sys_write(int fd, UserspacePointer<uint8_t> buffer, size_t count) {
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd])
		return -EBADF;
	ssize_t ret = _file_descriptors[fd]->write(buffer, count);
	return ret;
}

int Process::sys_lseek(int file, off_t off, int whence) {
	if(file < 0 || file >= (int) _file_descriptors.size() || !_file_descriptors[file])
		return -EBADF;
	return _file_descriptors[file]->seek(off, whence);
}

int Process::sys_open(UserspacePointer<char> filename, int options, int mode) {
	kstd::string path = filename.str();
	if(path == "/hello.c") {
		_cwd->inode()->find_id("hello.c");
		return -1;
	}
	mode &= 04777; //We just want the permission bits
	auto fd_or_err = VFS::inst().open(path, options, mode & (~_umask), _user, _cwd);
	if(fd_or_err.is_error())
		return fd_or_err.code();
	_file_descriptors.push_back(fd_or_err.value());
	fd_or_err.value()->set_owner(_self_ptr);
	fd_or_err.value()->set_path(path);
	fd_or_err.value()->set_id((int) _file_descriptors.size() - 1);
	return (int)_file_descriptors.size() - 1;
}

int Process::sys_close(int file) {
	if(file < 0 || file >= (int) _file_descriptors.size() || !_file_descriptors[file])
		return -EBADF;
	_file_descriptors[file] = kstd::Arc<FileDescriptor>(nullptr);
	return 0;
}