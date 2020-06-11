#ifndef DUCKOS_RANDOMDEVICE_H
#define DUCKOS_RANDOMDEVICE_H

#include "CharacterDevice.h"

class RandomDevice: public CharacterDevice {
public:
	RandomDevice();
	size_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
};


#endif //DUCKOS_RANDOMDEVICE_H
