#ifndef DUCKOS_CHARACTERDEVICE_H
#define DUCKOS_CHARACTERDEVICE_H

#include "Device.h"

class CharacterDevice: public Device {
public:
	CharacterDevice(unsigned major, unsigned minor);
	bool is_character_device() override;
};


#endif //DUCKOS_CHARACTERDEVICE_H
