

#include "NullDevice.h"

NullDevice::NullDevice(): CharacterDevice(1, 3) {

}

ssize_t NullDevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	return 0;
}

ssize_t NullDevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	return 0;
}
