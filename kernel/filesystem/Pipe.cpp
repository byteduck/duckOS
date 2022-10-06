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

#include "Pipe.h"
#include <kernel/tasking/Signal.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/filesystem/FileDescriptor.h>

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
	if(!_writers) {
		_blocker.set_ready(true);
	}
}

ssize_t Pipe::read(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	if(!_writers && _queue.empty()) return 0;
	if(!_blocker.is_ready())
		TaskManager::current_thread()->block(_blocker);
	LOCK(_lock);
	if(count > _queue.size())
		count = _queue.size();
	for(size_t i = 0; i < count; i++)
		buffer.set(i, _queue.pop_front());
	if(_queue.empty() && _writers)
		_blocker.set_ready(false);
	return count;
}

ssize_t Pipe::write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	//TODO: Block writes bigger than available space
	if(_readers == 0) {
		TaskManager::current_process()->kill(SIGPIPE);
		return -EPIPE;
	}

	LOCK(_lock);

	size_t nwrote = 0;
	bool flag = true;
	while(nwrote < count && flag) {
		flag = _queue.push_back(buffer.get(nwrote));
		nwrote++;
	}

	if(nwrote)
		_blocker.set_ready(true);

	return nwrote;
}

bool Pipe::is_fifo() {
	return true;
}

bool Pipe::can_read(const FileDescriptor& fd) {
	return !_queue.empty() && !fd.is_fifo_writer();
}
