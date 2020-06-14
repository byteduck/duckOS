#ifndef DUCKOS_KEYBOARDDEVICE_H
#define DUCKOS_KEYBOARDDEVICE_H

#include "CharacterDevice.h"
#include <kernel/keyboard.h>
#include <common/circular_queue.hpp>
#include <kernel/interrupt/IRQHandler.h>

class KeyEvent {
public:
	uint16_t scancode;
	uint8_t key;
	uint8_t character;
	uint8_t flags;
	bool pressed() const;
};

class KeyboardHandler {
public:
	virtual void handle_key(KeyEvent) = 0;
};

class KeyboardDevice: public CharacterDevice, public IRQHandler {
public:
	static KeyboardDevice* inst();

	KeyboardDevice();

	//Device
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;

	//IRQHandler
	void set_handler(KeyboardHandler* handler);
	void handle_irq(Registers* regs) override;

private:
	static KeyboardDevice* _instance;

	void set_mod(uint8_t mod, bool state);
	void set_key_state(uint8_t key, bool pressed);

	DC::circular_queue<KeyEvent> _event_buffer;
	KeyboardHandler* _handler = nullptr;
	uint8_t _modifiers = 0;
	bool _e0_flag = false;
};


#endif //DUCKOS_KEYBOARDDEVICE_H
