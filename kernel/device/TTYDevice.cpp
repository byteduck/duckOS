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
#include "TTYDevice.h"
#include "KeyboardDevice.h"

size_t TTYDevice::_current_tty;
DC::shared_ptr<TTYDevice> TTYDevice::_ttys[];

DC::shared_ptr<TTYDevice> TTYDevice::current_tty() {
	return _ttys[_current_tty];
}

void TTYDevice::set_current_tty(size_t tty) {
	DC::shared_ptr<TTYDevice> current = _ttys[_current_tty];
	if(current.get() != nullptr) current->_active = false;
	_current_tty = tty;
	KeyboardDevice::inst()->set_handler(_ttys[_current_tty].get());
}

void TTYDevice::register_tty(size_t id, TTYDevice *device) {
	if(_ttys[id]) PANIC("DUPLICATE_TTY", "A TTY tried to register with an ID that was already registered.", true);
	_ttys[id] = DC::shared_ptr<TTYDevice>(device);
}


TTYDevice::TTYDevice(size_t id, const DC::string& name, unsigned int major, unsigned int minor):
	CharacterDevice(major, minor), _name(name), _input_buffer(1024), _buffered_input_buffer(1024), _id(id)
{
	register_tty(id, this);
}

ssize_t TTYDevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	for(size_t i = 0; i < count; i++) putch(buffer[i]);
	return count;
}

ssize_t TTYDevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	if(buffered && _input_buffer.empty()) TaskManager::current_process()->yield_to(_buffer_yielder);
	count = min(count, _input_buffer.size());
	size_t count_loop = count;
	while(count_loop--) *buffer++ = _input_buffer.pop_front();
	return count;
}

bool TTYDevice::istty() {
	return true;
}


void TTYDevice::set_active() {
	set_current_tty(_id);
}

bool TTYDevice::active() {
	return _active;
}

void TTYDevice::handle_key(KeyEvent event) {
	if(!event.pressed()) return;
	if(!event.character) return;
	if(buffered) {
		if(event.character == '\n') {
			_buffered_input_buffer.push('\n');
			while (!_buffered_input_buffer.empty()) {
				_input_buffer.push(_buffered_input_buffer.pop_front());
			}
			putch('\n');
			_buffer_yielder.set_ready();
		} else if(event.character == '\b') {
			if(!_buffered_input_buffer.empty()){
				_buffered_input_buffer.pop_back();
				putch('\b');
			}
		} else {
			_buffered_input_buffer.push(event.character);
			putch(event.character);
		}
	} else {
		_input_buffer.push(event.character);
		putch(event.character);
	}
}