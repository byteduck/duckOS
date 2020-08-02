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

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#ifndef DUCKOS_FILEDESCRIPTOR_H
#define DUCKOS_FILEDESCRIPTOR_H

#include <common/shared_ptr.hpp>
#include "File.h"
#include "Inode.h"
#include "InodeMetadata.h"
#include "DirectoryEntry.h"
#include <kernel/tasking/YieldLock.h>
#include <kernel/tasking/Lock.h>

class File;
class DirectoryEntry;
class Device;
class FileDescriptor {
public:
	explicit FileDescriptor(DC::shared_ptr<File> file);
	explicit FileDescriptor(Device* device);
	explicit FileDescriptor(FileDescriptor& other);
	~FileDescriptor();

	void set_options(int options);
	bool readable();
	bool writable();
	bool append_mode();
	InodeMetadata metadata();
	File* file();

	int seek(off_t offset, int whence);
	ssize_t read(uint8_t* buffer, size_t count);
	ssize_t read_dir_entry(DirectoryEntry *buffer);
	ssize_t read_dir_entries(char *buffer, size_t len);
	ssize_t write(const uint8_t* buffer, size_t count);
	size_t offset();

	void set_fifo_reader();
	void set_fifo_writer();

private:
	File* _file;
	DC::shared_ptr<File> _fileptr;
	DC::shared_ptr<Inode> _inode;

	bool _readable {false};
	bool _writable {false};
	bool _can_seek {true};
	bool _append {false};

	off_t _seek {0};
	bool _is_fifo_writer = false;

	YieldLock lock;
};


#endif //DUCKOS_FILEDESCRIPTOR_H
