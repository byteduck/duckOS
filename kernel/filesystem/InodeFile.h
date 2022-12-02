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

#include "File.h"

class Inode;
class InodeFile: public File {
public:
	InodeFile(kstd::Arc<Inode>);

	bool is_inode() override;
	kstd::Arc<Inode> inode();
	ssize_t read(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;
	ssize_t read_dir_entry(FileDescriptor& fd, size_t offset, SafePointer<DirectoryEntry> buffer) override;
	ssize_t write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;
	void open(FileDescriptor& fd, int options) override;
	void close(FileDescriptor& fd) override;
	virtual bool can_read(const FileDescriptor& fd) override;
	virtual bool can_write(const FileDescriptor& fd) override;

private:
	kstd::Arc<Inode> _inode;
};


