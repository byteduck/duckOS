#include "CharacterDevice.h"

CharacterDevice::CharacterDevice(unsigned int major, unsigned int minor) : Device(major, minor) {

}

bool CharacterDevice::is_character_device() {
	return true;
}
