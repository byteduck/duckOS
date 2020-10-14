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

#ifndef DUCKOS_INODEFILE_H
#define DUCKOS_INODEFILE_H

#include "File.h"

class InodeFile: public File {
public:
	InodeFile(DC::shared_ptr<Inode>);

	bool is_inode() override;
	DC::shared_ptr<Inode> inode();
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	ssize_t read_dir_entry(FileDescriptor& fd, size_t offset, DirectoryEntry* buffer) override;
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
	void open(FileDescriptor& fd, int options) override;
	void close(FileDescriptor& fd) override;
	virtual bool can_read(const FileDescriptor& fd);
	virtual bool can_write(const FileDescriptor& fd);

private:
	DC::shared_ptr<Inode> _inode;
};


#endif //DUCKOS_INODEFILE_H
