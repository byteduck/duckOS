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

#include <common/defines.h>
#include <kernel/terminal/PTYMuxDevice.h>
#include "Device.h"
#include "ZeroDevice.h"
#include "RandomDevice.h"
#include "NullDevice.h"
#include "KeyboardDevice.h"
#include "MouseDevice.h"

DC::vector<DC::shared_ptr<Device>> Device::_devices;
SpinLock Device::_lock;

void Device::init() {
	new (&_devices) DC::vector<DC::shared_ptr<Device>>();
	new (&_lock) SpinLock();
	new ZeroDevice();
	new RandomDevice();
	new NullDevice();
	new KeyboardDevice();
	new MouseDevice();
	new PTYMuxDevice();
}

Device::Device(unsigned major, unsigned minor): _major(major), _minor(minor) {
	LOCK(_lock);
	auto res = get_device(_major, _minor);
	if (res.is_error() && res.code() == -ENODEV) {
		_devices.push_back(DC::shared_ptr<Device>(this));
		printf("[Device] Device %d,%d registered\n", _major, _minor);
	} else {
		printf("[Device] Tried to register already-registered device %d,%d!\n", _major, _minor);
	}
}

Device::~Device() {
}

unsigned Device::major() {
	return _major;
}

unsigned Device::minor() {
	return _minor;
}


DC::vector<DC::shared_ptr<Device>> Device::devices() {
	return _devices;
}

ResultRet<DC::shared_ptr<Device>> Device::get_device(unsigned major, unsigned minor) {
	LOCK(_lock);
	for(size_t i = 0; i < _devices.size(); i++) {
		if(_devices[i]){
			if(_devices[i]->_major == major && _devices[i]->_minor == minor)
				return _devices[i];
		}
	}
	return -ENODEV;
}

void Device::remove_device(unsigned major, unsigned minor) {
	LOCK(_lock);
	for(auto i = 0; i < _devices.size(); i++) {
		if(_devices[i]){
			if(_devices[i]->_major == major && _devices[i]->_minor == minor){
				_devices.erase(i);
				printf("[Device] Device %d,%d deregistered\n", major, minor);
			}
		}
	}
}

bool Device::is_block_device() {
	return false;
}

bool Device::is_character_device() {
	return false;
}
