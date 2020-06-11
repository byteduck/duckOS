#ifndef DUCKOS_ZERODEVICE_H
#define DUCKOS_ZERODEVICE_H

#include "CharacterDevice.h"

class ZeroDevice: public CharacterDevice {
public:
	ZeroDevice();
	size_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	size_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
};


#endif //DUCKOS_ZERODEVICE_H
