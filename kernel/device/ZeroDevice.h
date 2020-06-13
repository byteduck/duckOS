#ifndef DUCKOS_ZERODEVICE_H
#define DUCKOS_ZERODEVICE_H

#include "CharacterDevice.h"

class ZeroDevice: public CharacterDevice {
public:
	ZeroDevice();
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
};


#endif //DUCKOS_ZERODEVICE_H
