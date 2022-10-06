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

#pragma once

#include <kernel/memory/MemoryManager.h>
#include <kernel/filesystem/File.h>
#include <kernel/kstd/circular_queue.hpp>
#include <kernel/tasking/SpinLock.h>

#define PIPE_SIZE PAGE_SIZE

class Pipe: public File {
public:
	//Pipe
	Pipe();
	~Pipe();

	void add_reader();
	void add_writer();
	void remove_reader();
	void remove_writer();

	//File
	ssize_t read(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;
	ssize_t write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) override;
	bool is_fifo() override;
	bool can_read(const FileDescriptor& fd) override;

private:
	kstd::circular_queue<uint8_t> _queue;
	size_t _readers = 0;
	size_t _writers = 0;
	BooleanBlocker _blocker;
	SpinLock _lock;
};


