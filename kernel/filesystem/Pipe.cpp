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

#include <kernel/tasking/Signal.h>
#include <kernel/tasking/TaskManager.h>
#include <common/defines.h>
#include "Pipe.h"

Pipe::Pipe(): _queue(PIPE_SIZE) {}

Pipe::~Pipe() = default;

void Pipe::add_reader() {
	_readers++;
}

void Pipe::add_writer() {
	_writers++;
}

void Pipe::remove_reader() {
	_readers--;
}

void Pipe::remove_writer() {
	_writers--;
}

ssize_t Pipe::read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) {
	if(_writers == 0 || _queue.empty()) return 0;
	if(count > _queue.size()) count = _queue.size();
	for(size_t i = 0; i < count; i++) buffer[i] = _queue.pop_front();
	return count;
}

ssize_t Pipe::write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) {
	if(_readers == 0) {
		TaskManager::current_process()->kill(SIGPIPE);
		return -EPIPE;
	}

	size_t nwrote = 0;
	bool flag = true;
	while(nwrote < count && flag) {
		flag = _queue.push(buffer[nwrote]);
		nwrote++;
	}

	return nwrote;
}

bool Pipe::is_fifo() {
	return true;
}

void Pipe::open(FileDescriptor& fd, int options) {
	if(fd.is_fifo_writer())
		add_writer();
	else
		add_reader();
}

void Pipe::close(FileDescriptor& fd) {
	if(fd.is_fifo_writer())
		remove_writer();
	else
		remove_reader();
}
