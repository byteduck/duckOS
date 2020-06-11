#ifndef DUCKOS_DEVICE_H
#define DUCKOS_DEVICE_H

#include <kernel/filesystem/File.h>
#include <common/vector.hpp>

class Device: public File {
public:
	static void init();
	static DC::vector<Device*> devices();
	static Device* get_device(unsigned major, unsigned minor);
	static void remove_device(unsigned major, unsigned minor);
	static void add_device(Device* dev);


	Device(unsigned major, unsigned minor);
	virtual ~Device() override;

	unsigned major();
	unsigned minor();

	virtual bool is_block_device();
	virtual bool is_character_device();
private:
	static DC::vector<Device*> _devices;

	unsigned _major = 0;
	unsigned _minor = 0;
};

#endif //DUCKOS_DEVICE_H
