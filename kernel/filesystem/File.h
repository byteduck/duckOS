/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#pragma once

#include <kernel/kstd/Arc.h>
#include <kernel/Result.hpp>
#include <kernel/memory/SafePointer.h>

class FileDescriptor;
class DirectoryEntry;
class File {
public:
	virtual ~File();
	virtual bool is_inode();
	virtual ssize_t read(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count);
	virtual ssize_t write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count);
	virtual ssize_t read_dir_entries(FileDescriptor& fd, size_t bufsz, SafePointer<uint8_t> buffer);
	virtual bool is_tty();
	virtual bool is_pty_controller();
	virtual bool is_pty_mux();
	virtual bool is_pty();
	virtual bool is_fifo();
	virtual int ioctl(unsigned request, SafePointer<void*> argp);
	virtual void open(FileDescriptor& fd, int options);
	virtual void close(FileDescriptor& fd);
	virtual bool can_read(const FileDescriptor& fd);
	virtual bool can_write(const FileDescriptor& fd);
protected:
	File();
};


