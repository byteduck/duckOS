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


DC::vector<Device *> Device::devices() {
	return _devices;
}

Device *Device::get_device(size_t major, size_t minor) {
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
