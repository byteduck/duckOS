#ifndef DUCKOS_NULLDEVICE_H
#define DUCKOS_NULLDEVICE_H

#include "CharacterDevice.h"

class NullDevice: public CharacterDevice {
public:
	NullDevice();
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
};


#endif //DUCKOS_NULLDEVICE_H
