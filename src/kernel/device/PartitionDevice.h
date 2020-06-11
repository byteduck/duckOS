#ifndef DUCKOS_PARTITIONDEVICE_H
#define DUCKOS_PARTITIONDEVICE_H

#include <kernel/device/BlockDevice.h>

class PartitionDevice: public BlockDevice {
public:
	PartitionDevice(unsigned major, unsigned minor, const DC::shared_ptr<BlockDevice>& parent, size_t offset_blocks);
	bool read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) override;
	bool write_blocks(uint32_t block, uint32_t count, uint8_t *buffer) override;
	size_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	size_t block_size() override;
	size_t part_offset();
	DC::shared_ptr<File> parent();
private:
	uint32_t _offset;
	DC::shared_ptr<BlockDevice> _parent;
};


#endif //DUCKOS_PARTITIONDEVICE_H
