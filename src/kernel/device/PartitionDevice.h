#ifndef DUCKOS_PARTITIONDEVICE_H
#define DUCKOS_PARTITIONDEVICE_H

#include <kernel/device/BlockDevice.h>

class PartitionDevice: public BlockDevice {
public:
	uint32_t offset;
	BlockDevice* parent;

	PartitionDevice(BlockDevice* parent, uint32_t offset);
	bool read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) override;
	bool write_blocks(uint32_t block, uint32_t count, uint8_t *buffer) override;
	size_t block_size() override;
};


#endif //DUCKOS_PARTITIONDEVICE_H
