#include <kernel/kstdio.h>
#include <common/defines.h>
#include "BlockDevice.h"

BlockDevice::BlockDevice() {
}

bool BlockDevice::read_block(uint32_t block, uint8_t *buffer) {
	return this->read_blocks(block, 1, buffer);
}

bool BlockDevice::write_block(uint32_t block, uint8_t *buffer) {
	return this->write_blocks(block, 1, buffer);
}

bool BlockDevice::read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	return false;
}

bool BlockDevice::write_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	return false;
}

size_t BlockDevice::block_size() {
	return 0;
}
