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

#include <kernel/tasking/TaskManager.h>
#include <kernel/device/VGADevice.h>
#include <kernel/kstd/defines.h>
#include "TTYDevice.h"




TTYDevice::TTYDevice(unsigned int major, unsigned int minor): CharacterDevice(major, minor), _input_buffer(1024), _buffered_input_buffer(1024) {

}

ssize_t TTYDevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	return tty_write(buffer, count);
}

ssize_t TTYDevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	LOCK(_input_lock);
	while(buffered && _input_buffer.empty())
		TaskManager::current_process()->block(_buffer_blocker);
	count = min(count, _input_buffer.size());
	size_t count_loop = count;
	while(count_loop--) *buffer++ = _input_buffer.pop_front();
	if(buffered && _input_buffer.empty()) _buffer_blocker.set_ready(false);
	return count;
}

bool TTYDevice::is_tty() {
	return true;
}

void TTYDevice::emit(uint8_t c) {
	//Check if we should generate a signal
	if(c == '\x03') {
		generate_signal(SIGINT);
		return;
	}

	if(buffered) {
		if(c == '\n') {
			_buffered_input_buffer.push('\n');
			while (!_buffered_input_buffer.empty()) {
				_input_buffer.push(_buffered_input_buffer.pop_front());
			}
			echo(c);
			_buffer_blocker.set_ready(true);
		} else if(c == '\b') {
			if(!_buffered_input_buffer.empty()){
				_buffered_input_buffer.pop_back();
				echo(c);
			}
		} else {
			_buffered_input_buffer.push(c);
			echo(c);
		}
	} else {
		_input_buffer.push(c);
		echo(c);
	}
}

bool TTYDevice::can_read(const FileDescriptor& fd) {
	return !_input_buffer.empty();
}

bool TTYDevice::can_write(const FileDescriptor& fd) {
	return true;
}

int TTYDevice::ioctl(unsigned int request, void* argp) {
	auto* cur_proc = TaskManager::current_process();
	switch(request) {
		case TIOCSCTTY:
			cur_proc->set_tty(shared_ptr());
			return SUCCESS;
		case TIOCGPGRP:
			return _pgid;
		case TIOCSPGRP: {
			auto pgid = (pid_t) argp;
			if(pgid <= 0)
				return -EINVAL;
			auto* proc = TaskManager::process_for_pgid(pgid);
			if(!proc)
				return -EINVAL;
			if(cur_proc->sid() != proc->sid())
				return -EPERM;
			_pgid = pgid;
			return SUCCESS;
		}

		default:
			return -EINVAL;
	}
}

void TTYDevice::generate_signal(int sig) {
	if(_pgid == 0)
		return;
	TaskManager::kill_pgid(_pgid, sig);
}
