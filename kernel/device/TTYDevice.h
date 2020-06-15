#ifndef DUCKOS_TTYDEVICE_H
#define DUCKOS_TTYDEVICE_H

#include <common/circular_queue.hpp>
#include "CharacterDevice.h"
#include "KeyboardDevice.h"

#define NUM_TTYS 8

class TTYDevice: public CharacterDevice, public KeyboardHandler {
public:
	static DC::shared_ptr<TTYDevice> current_tty();

	TTYDevice(size_t id, const DC::string& name, unsigned major, unsigned minor);
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	void set_active();
	bool active();

protected:
	static void register_tty(size_t id, TTYDevice* device);
	static void set_current_tty(size_t tty);

private:
	static size_t _current_tty;
	static DC::shared_ptr<TTYDevice> _ttys[NUM_TTYS];

	void handle_key(KeyEvent) override;

	DC::circular_queue<uint8_t> _input_buffer;
	DC::string _name;
	size_t _id;
	bool _active = false;
};

#endif //DUCKOS_TTYDEVICE_H
