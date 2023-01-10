/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/VFS.h"
#include "../filesystem/Pipe.h"

int Process::sys_pipe(UserspacePointer<int> filedes, int options) {
	options &= (O_CLOEXEC | O_NONBLOCK);

	//Make the pipe
	auto pipe = kstd::make_shared<Pipe>();
	pipe->add_reader();
	pipe->add_writer();

	//Make the read FD
	auto pipe_read_fd = kstd::make_shared<FileDescriptor>(pipe);
	pipe_read_fd->set_owner(_self_ptr);
	pipe_read_fd->set_options(O_RDONLY | options);
	pipe_read_fd->set_fifo_reader();
	_file_descriptors.push_back(pipe_read_fd);
	pipe_read_fd->set_id((int) _file_descriptors.size() - 1);
	filedes.set(0, (int) _file_descriptors.size() - 1);

	//Make the write FD
	auto pipe_write_fd = kstd::make_shared<FileDescriptor>(pipe);
	pipe_write_fd->set_owner(_self_ptr);
	pipe_write_fd->set_options(O_WRONLY | options);
	pipe_write_fd->set_fifo_writer();
	_file_descriptors.push_back(pipe_write_fd);
	pipe_write_fd->set_id((int) _file_descriptors.size() - 1);
	filedes.set(1, (int) _file_descriptors.size() - 1);

	return SUCCESS;
}