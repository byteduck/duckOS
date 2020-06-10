#include <kernel/kstdio.h>
#include "PartitionDevice.h"

PartitionDevice::PartitionDevice(const DC::shared_ptr<BlockDevice> &parent, uint32_t offset_blocks):
_parent(parent), _offset(offset_blocks * parent->block_size()) {

}

bool PartitionDevice::read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	return _parent->read_blocks(block + _offset, count, buffer);
}

bool PartitionDevice::write_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	return _parent->write_blocks(block + _offset, count, buffer);
}

size_t PartitionDevice::read(FileDescriptor &fd, size_t start, uint8_t *buffer, size_t count) {
	return _parent->read(fd, start + _offset, buffer, count);
}

size_t PartitionDevice::block_size() {
	return _parent->block_size();
}

size_t PartitionDevice::part_offset() {
	return _offset;
}

DC::shared_ptr<File> PartitionDevice::parent() {
	return _parent;
}