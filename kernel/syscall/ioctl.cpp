/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/FileDescriptor.h"

int Process::sys_ioctl(int fd, unsigned request, UserspacePointer<void*> argp) {
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd])
		return -EBADF;
	return _file_descriptors[fd]->ioctl(request, argp);
}