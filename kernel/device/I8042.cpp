/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include "I8042.h"
#include <kernel/IO.h>
#include <kernel/kstd/kstdio.h>
#include "KeyboardDevice.h"
#include "MouseDevice.h"

I8042* I8042::_inst = nullptr;

I8042& I8042::inst() {
	if(!_inst)
		_inst = new I8042();
	ASSERT(_inst);
	return *_inst;
}

bool I8042::init() {
	if(_inst) {
		printf("[I8042] Already initialized!\n");
		return false;
	}

	inst();

	printf("[I8042] Attempting to initialize...\n");

	//Disable both ports (Port 2 ignored if not present)
	controller_command(I8042_CMD_DISABLE_PORT1);
	controller_command(I8042_CMD_DISABLE_PORT2);

	//Drain the buffer
	while(IO::inb(I8042_STATUS) & I8042_STATUS_OUTPUT_FULL)
		IO::inb(I8042_BUFFER);

	//Read the configuration, disable IRQs, write the configuration
	uint8_t config = controller_command_read(I8042_CMD_READ_CONFIG);
	config &= ~(I8042_CONFIG_INT1 | I8042_CONFIG_INT2);
	write_config(config);

	//TODO: Self-test doesn't seem to work on real hardware
	//Perform self-test and restore configuration
	/*uint8_t self_test = controller_command_read(I8042_CMD_SELF_TEST);
	if(self_test != I8042_SELF_TEST_SUCCESSFUL) {
		printf("[I8042] Self-test failed (Returned 0x%x)\n", self_test);
		return false;
	}
	write_config(config);*/

	bool dual_channel = config & I8042_CONFIG_CLOCK2;
	printf("[I8042] Found %s-channel controller\n", dual_channel ? "dual" : "single");

	bool keyboard_available = false, mouse_available = false;

	//Test for the availability of the keyboard
	if(!controller_command_read(I8042_CMD_TEST_PORT1)) {
		printf("[I8042] Keyboard available\n");
		controller_command(I8042_CMD_ENABLE_PORT1);
		config |= I8042_CONFIG_INT1;
		config &= ~I8042_CONFIG_CLOCK1;
		keyboard_available = true;
	}
	else {
		printf("[I8042] Keyboard unavailable\n");
	}

	//Test for the availability of the mouse
	if(dual_channel) {
		if(!controller_command_read(I8042_CMD_TEST_PORT2)) {
			printf("[I8042] Mouse available\n");
			controller_command(I8042_CMD_ENABLE_PORT2);
			config |= I8042_CONFIG_INT2;
			config &= ~I8042_CONFIG_CLOCK2;
			mouse_available = true;
		}
		else {
			printf("[I8042] Mouse unavailable\n");
		}
	}

	//Reset the devices
	if(keyboard_available) {
		if(reset_device(KEYBOARD)) {
			printf("[I8042] Keyboard successfully enabled\n");
			_inst->_keyboard = new KeyboardDevice();
		} else {
			printf("[I8042] Error enabling keyboard\n");
			_inst->_keyboard = nullptr;
		}
	}

	if(mouse_available) {
		if(reset_device(MOUSE)) {
			printf("[I8042] Mouse successfully enabled\n");
			_inst->_mouse = new MouseDevice();
		} else {
			printf("[I8042] Error enabling mouse\n");
			_inst->_mouse = nullptr;
		}
	}

	//Write the updated config
	write_config(config);

	return true;
}

void I8042::handle_irq() {
	controller_command(I8042_CMD_DISABLE_PORT1);
	controller_command(I8042_CMD_DISABLE_PORT2);

	uint8_t status = IO::inb(I8042_STATUS);
	if(!(status & I8042_STATUS_OUTPUT_FULL))
		return;

	uint8_t byte = IO::inb(I8042_BUFFER);
	if((status & I8042_STATUS_WHICH_BUFFER) == I8042_KEYBOARD_BUFFER) {
		if(!_keyboard) {
			printf("[I8042] Received keyboard buffer data, but no keyboard device is present!\n");
			return;
		}
		_keyboard->handle_byte(byte);
	} else {
		if(!_mouse) {
			printf("[I8042] Received mouse buffer data, but no mouse device is present!\n");
			return;
		}
		_mouse->handle_byte(byte);
	}

	controller_command(I8042_CMD_ENABLE_PORT1);
	controller_command(I8042_CMD_ENABLE_PORT2);
}

void I8042::controller_command(uint8_t command) {
	wait_write();
	IO::outb(I8042_STATUS, command);
}

uint8_t I8042::controller_command_read(uint8_t command) {
	controller_command(command);
	uint32_t timeout = 100000;
	while(--timeout) {
		if(IO::inb(I8042_STATUS) & I8042_STATUS_OUTPUT_FULL)
			return IO::inb(I8042_BUFFER);
	}
	printf("[I8042] Controller read timed out...\n");
	return 0;
}

void I8042::write_config(uint8_t config) {
	controller_command(I8042_CMD_WRITE_CONFIG);
	wait_write();
	IO::outb(I8042_BUFFER, config);
}

bool I8042::reset_device(I8042::DeviceType device) {
	write(device, I8042_CMD_RESET);
	if(read(device) != I8042_ACK)
		return false;
	return read(device) == I8042_RESET_SUCCESSFUL;
}

void I8042::write(I8042::DeviceType type, uint8_t value) {
	if(type == MOUSE) {
		//We have to tell the controller we want to talk to the second port
		wait_write();
		IO::outb(I8042_STATUS, 0xD4);
	}
	wait_write();
	IO::outb(I8042_BUFFER, value);
}

void I8042::wait_write() {
	uint32_t timeout = 1000;
	while(--timeout) {
		if(!(IO::inb(I8042_STATUS) & I8042_STATUS_INPUT_FULL))
			return;
	}
	printf("[I8042] Write timed out...\n");
}

uint8_t I8042::read(I8042::DeviceType type) {
	wait_read(type);
	return IO::inb(I8042_BUFFER);
}

void I8042::wait_read(DeviceType type) {
	uint32_t timeout = 1000;
	uint8_t status;
	uint8_t buf = type == MOUSE ? I8042_MOUSE_BUFFER : I8042_KEYBOARD_BUFFER;
	while(--timeout) {
		status = IO::inb(I8042_STATUS);
		if((status & I8042_STATUS_OUTPUT_FULL) && ((status & I8042_STATUS_WHICH_BUFFER) == buf))
			return;
	}
	printf("[I8042] Read timed out...\n");
}
