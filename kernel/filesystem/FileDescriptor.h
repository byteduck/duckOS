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

#include <kernel/kstd/shared_ptr.hpp>
#include <kernel/kstd/string.h>
#include <kernel/tasking/SpinLock.h>
#include <kernel/kstd/unix_types.h>
#include "File.h"

class DirectoryEntry;
class Device;
class InodeMetadata;
class Inode;
class FileDescriptor {
public:
	explicit FileDescriptor(const kstd::shared_ptr<File>& file, Process* owner = nullptr);
	FileDescriptor(FileDescriptor& other, Process* new_owner = nullptr);
	~FileDescriptor();

	void set_options(int options);
	void unset_options(int options);
	bool readable() const;
	bool writable() const;
	bool append_mode() const;
	bool nonblock() const;
	bool cloexec() const;
	InodeMetadata metadata();
	kstd::shared_ptr<File> file();
	void open();
	pid_t owner() const;
	void set_owner(Process* owner);
	void set_owner(pid_t owner);
	void set_path(const kstd::string& path);
	kstd::string path();
	void set_id(int id);
	int id();

	int seek(off_t offset, int whence);
	ssize_t read(uint8_t* buffer, size_t count);
	ssize_t read_dir_entry(DirectoryEntry *buffer);
	ssize_t read_dir_entries(char *buffer, size_t len);
	ssize_t write(const uint8_t* buffer, size_t count);
	size_t offset() const;
	int ioctl(unsigned request, void* argp);

	void set_fifo_reader();
	void set_fifo_writer();
	bool is_fifo_writer() const;

private:
	kstd::shared_ptr<File> _file;
	kstd::shared_ptr<Inode> _inode;
	pid_t _owner = -1;
	kstd::string _path = "";
	int _id = -1;

	bool _readable {false};
	bool _writable {false};
	bool _can_seek {true};
	bool _append {false};
	int _options = 0;

	off_t _seek {0};
	bool _is_fifo_writer = false;

	SpinLock lock;
};


