#include "TTYDevice.h"

TTYDevice::TTYDevice(unsigned int major, unsigned int minor) : CharacterDevice(major, minor) {

}

size_t TTYDevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	for(auto i = 0; i < count; i++) putch(buffer[i]);
	return count;
}

size_t TTYDevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	return 0;
}
