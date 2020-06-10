#ifndef DUCKOS_DEVICE_H
#define DUCKOS_DEVICE_H

#include <kernel/filesystem/File.h>

class Device: public File {
public:
	Device();
};

#endif //DUCKOS_DEVICE_H
