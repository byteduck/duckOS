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

#include "PTYDevice.h"
#include "PTYControllerDevice.h"

PTYControllerDevice::PTYControllerDevice(unsigned int id): CharacterDevice(300, id), _output_buffer(8192) {
	_pty = (new PTYDevice(id, shared_ptr()))->shared_ptr();
	_buffer_blocker.set_ready(true);
}

ssize_t PTYControllerDevice::write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	LOCK_N(_write_lock, write_locker);
	LOCK_N(_ref_lock, ref_locker);
	if(!_pty)
		return 0;
	int off = 0;
	while(count--)
		_pty->emit(buffer.get(off++));
	return count;
}

ssize_t PTYControllerDevice::read(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	LOCK(_output_lock);
	count = min(count, _output_buffer.size());
	size_t count_loop = count;
	int off = 0;
	while(count_loop--)
		buffer.set(off++, _output_buffer.pop_front());
	_buffer_blocker.set_ready(_output_buffer.empty());
	return count;
}

bool PTYControllerDevice::is_pty_controller() {
	return true;
}

bool PTYControllerDevice::can_read(const FileDescriptor& fd) {
	return !_output_buffer.empty();
}

bool PTYControllerDevice::can_write(const FileDescriptor& fd) {
	LOCK(_ref_lock);
	return _pty.get() != nullptr;
}

int PTYControllerDevice::ioctl(unsigned int request, SafePointer<void*> argp) {
	if(!_pty)
		return -EIO;

	switch(request) {
		case TIOCSWINSZ:
		case TIOCGPGRP:
			return _pty->ioctl(request, argp);

		default:
			return -EINVAL;
	}
}

size_t PTYControllerDevice::putchars(const uint8_t* buffer, size_t count) {
	if(count > _output_buffer.capacity())
		return -ENOSPC;

	// Acquire lock and wait for space in buffer
	while(true) {
		_output_lock.acquire();
		if(_output_buffer.capacity() - _output_buffer.size() < count) {
			_output_lock.release();
			TaskManager::current_thread()->block(_buffer_blocker);
		} else {
			break;
		}
	}

	size_t count_loop = count;
	while(count_loop--)
		_output_buffer.push_back(*(buffer++));

	_output_lock.release();

	return count;
}

void PTYControllerDevice::notify_pty_closed() {
	_pty = kstd::shared_ptr<PTYDevice>(nullptr);
}

void PTYControllerDevice::ref_inc() {
	LOCK(_ref_lock);
	num_refs++;
}

void PTYControllerDevice::ref_dec() {
	LOCK(_ref_lock);
	num_refs--;
	if(!num_refs) {
		if(_pty)
			_pty->notify_controller_closed();
		Device::remove_device(major(), minor());
	}
}

kstd::shared_ptr<PTYDevice> PTYControllerDevice::pty() {
	return _pty;
}
