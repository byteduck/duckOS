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

bool BlockDevice::read(uint32_t start, uint32_t length, uint8_t *buffer) {
	if(start % block_size() != 0 || length % block_size() != 0) return false;
	return this->read_blocks(start/block_size(), length/block_size(), buffer);
}

bool BlockDevice::write(uint32_t start, uint32_t length, uint8_t *buffer) {
	if(start % block_size() != 0 || length % block_size() != 0) return false;
	return this->write_blocks(start/block_size(), length/block_size(), buffer);
}

size_t BlockDevice::block_size() {
	return 0;
}
