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

#pragma once

#include <kernel/kstd/types.h>

//Ports
#define I8042_BUFFER 0x60u
#define I8042_STATUS 0x64u

//Status Register
#define I8042_STATUS_OUTPUT_FULL 0x01u
#define I8042_STATUS_INPUT_FULL 0x02u
#define I8042_STATUS_SYSTEM_FLAG 0x04u
#define I8042_STATUS_COMMAND_DATA 0x08u
#define I8042_STATUS_WHICH_BUFFER 0x20u
#define I8042_STATUS_TIMEOUT_ERR 0x40u
#define I8042_STATUS_PARITY_ERR 0x80u
#define I8042_MOUSE_BUFFER 0x20u
#define I8042_KEYBOARD_BUFFER 0x00u

//Controller Commands
#define I8042_CMD_READ_CONFIG 0x20u
#define I8042_CMD_WRITE_CONFIG 0x60u
#define I8042_CMD_DISABLE_PORT2 0xA7u
#define I8042_CMD_ENABLE_PORT2 0xA8u
#define I8042_CMD_TEST_PORT2 0xA9u
#define I8042_CMD_SELF_TEST 0xAAu
#define I8042_CMD_TEST_PORT1 0xABu
#define I8042_CMD_DISABLE_PORT1 0xADu
#define I8042_CMD_ENABLE_PORT1 0xAEu

//Device commands
#define I8042_CMD_RESET 0xFFu

//Config
#define I8042_CONFIG_INT1 0x01u
#define I8042_CONFIG_INT2 0x02u
#define I8042_CONFIG_SYS 0x04u
#define I8042_CONFIG_ZERO1 0x08u
#define I8042_CONFIG_CLOCK1 0x10u
#define I8042_CONFIG_CLOCK2 0x20u
#define I8042_CONFIG_TRANSLATION 0x40u
#define I8042_CONFIG_ZERO2 0x80u

//Misc
#define I8042_SELF_TEST_SUCCESSFUL 0x55u
#define I8042_RESET_SUCCESSFUL 0xAAu
#define I8042_ACK 0xFAu

class KeyboardDevice;
class MouseDevice;
class I8042 {
public:
	enum DeviceType {
		KEYBOARD,
		MOUSE
	};

	static I8042& inst();
	static bool init();

	void handle_irq();

private:
	friend class KeyboardDevice;
	friend class MouseDevice;

	I8042() = default;
	static I8042* _inst;

	static void controller_command(uint8_t command);
	static uint8_t controller_command_read(uint8_t command);
	static void write_config(uint8_t config);
	static bool reset_device(DeviceType device);

	static void write(DeviceType type, uint8_t value);
	static void wait_write();

	static uint8_t read(DeviceType type);
	static void wait_read(DeviceType type);

	KeyboardDevice* _keyboard;
	MouseDevice* _mouse;
};


