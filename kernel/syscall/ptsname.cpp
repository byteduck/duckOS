/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/FileDescriptor.h"
#include "../terminal/PTYControllerDevice.h"
#include "../terminal/PTYDevice.h"

int Process::sys_ptsname(int fd, UserspacePointer<char> buf, size_t bufsize) {
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd])
		return -EBADF;
	auto file = _file_descriptors[fd];

	//Check to make sure it's a PTY Controller
	if(!file->file()->is_pty_controller())
		return -ENOTTY;

	//Make sure the name isn't too long
	auto name = ((PTYControllerDevice*) file->file().get())->pty()->name();
	if(name.length() + 1 > bufsize)
		return -ERANGE;

	//Copy the name into the buffer and return success
	buf.write(name.c_str(), name.length() + 1);
	return SUCCESS;
}