#include <common/stdlib.h>
#include "ZeroDevice.h"

ZeroDevice::ZeroDevice(): CharacterDevice(1, 5) {

}

ssize_t ZeroDevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	size_t read = min(count, 512);
	memset(buffer, 0, read);
	return read;
}

ssize_t ZeroDevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	return count;
}
