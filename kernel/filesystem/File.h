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

#ifndef DUCKOS_FILE_H
#define DUCKOS_FILE_H

#include <common/shared_ptr.hpp>
#include <kernel/Result.hpp>
#include "FileDescriptor.h"
#include "DirectoryEntry.h"

class FileDescriptor;
class File {
public:
	virtual ~File();
	virtual bool is_inode();
	virtual ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count);
	virtual ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count);
	virtual ssize_t read_dir_entry(FileDescriptor& fd, size_t offset, DirectoryEntry* buffer);
	virtual bool is_tty();
	virtual bool is_fifo();
	virtual int ioctl(unsigned request, void* argp);
protected:
	File();
};


#endif //DUCKOS_FILE_H
