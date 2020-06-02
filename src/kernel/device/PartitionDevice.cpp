#include "PartitionDevice.h"

PartitionDevice::PartitionDevice(BlockDevice *parent, uint32_t offset) {
	this->parent = parent;
	this->offset = offset;
}

bool PartitionDevice::read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	return parent->read_blocks(block + offset, count, buffer);
}

bool PartitionDevice::write_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	return parent->write_blocks(block + offset, count, buffer);
}

size_t PartitionDevice::block_size() {
	return parent->block_size();
}
