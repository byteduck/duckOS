#include <common/stdlib.h>
#include <kernel/random.h>
#include "RandomDevice.h"

RandomDevice::RandomDevice(): CharacterDevice(1, 8) {

}

ssize_t RandomDevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	size_t amount = min(count, 512);
	get_random_bytes(buffer, amount);
	return amount;
}

ssize_t RandomDevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	return count;
}
