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

#include "Device.h"
#include "ZeroDevice.h"
#include "RandomDevice.h"
#include "NullDevice.h"

DC::vector<Device*> Device::_devices;

void Device::init() {
	_devices = DC::vector<Device*>();
	new ZeroDevice();
	new RandomDevice();
	new NullDevice();
}

Device::Device(unsigned major, unsigned minor): _major(major), _minor(minor) {
	add_device(this);
}

Device::~Device() {
	remove_device(_major, _minor);
}

unsigned Device::major() {
	return _major;
}

unsigned Device::minor() {
	return _major;
}


DC::vector<Device*> Device::devices() {
	return _devices;
}

Device *Device::get_device(unsigned major, unsigned minor) {
	for(auto i = 0; i < _devices.size(); i++) {
		if(_devices[i] != nullptr){
			if(_devices[i]->_major == major && _devices[i]->_minor == minor) return _devices[i];
		}
	}
	return nullptr;
}

void Device::remove_device(unsigned major, unsigned minor) {
	for(auto i = 0; i < _devices.size(); i++) {
		if(_devices[i] != nullptr){
			if(_devices[i]->_major == major && _devices[i]->_minor == minor){
				_devices.erase(i);
				printf("device: Device %d,%d deregistered\n", major, minor);
			}
		}
	}
}

void Device::add_device(Device* dev) {
	if(get_device(dev->_major, dev->_minor)) PANIC("DEVICE_DUPLICATE", "A device tried to register, but a device with the same major/minor was already regsitered.", true);
	_devices.push_back(dev);
	printf("device: Device %d,%d registered\n", dev->_major, dev->_minor);
}

bool Device::is_block_device() {
	return false;
}

bool Device::is_character_device() {
	return false;
}
