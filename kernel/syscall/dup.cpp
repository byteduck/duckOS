/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/FileDescriptor.h"

int Process::sys_dup(int oldfd) {
	if(oldfd < 0 || oldfd >= (int) _file_descriptors.size() || !_file_descriptors[oldfd])
		return -EBADF;
	auto new_fd = kstd::make_shared<FileDescriptor>(*_file_descriptors[oldfd]);
	_file_descriptors.push_back(new_fd);
	new_fd->set_id((int) _file_descriptors.size() - 1);
	new_fd->unset_options(O_CLOEXEC);
	return (int) _file_descriptors.size() - 1;
}

int Process::sys_dup2(int oldfd, int newfd) {
	if(oldfd < 0 || oldfd >= (int) _file_descriptors.size() || !_file_descriptors[oldfd])
		return -EBADF;
	if(newfd == oldfd)
		return oldfd;
	if(newfd >= _file_descriptors.size())
		_file_descriptors.resize(newfd + 1);
	auto new_fd = kstd::make_shared<FileDescriptor>(*_file_descriptors[oldfd]);
	new_fd->set_id(newfd);
	_file_descriptors[newfd] = new_fd;
	new_fd->unset_options(O_CLOEXEC);
	return newfd;
}