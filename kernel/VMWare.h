/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "Result.hpp"
#include "kernel/device/MouseDevice.h"

/* Thanks, OSDev Wiki! */

#define VMW_MAGIC  0x564D5868
#define VMW_PORT   0x5658
#define VMW_PORTHB 0x5659

#define VMW_ABSPOINTER_DATA    39
#define VMW_ABSPOINTER_STATUS  40
#define VMW_ABSPOINTER_COMMAND 41

#define VMW_ABSPOINTER_ERR      0xFFFF0000
#define VMW_ABSPOINTER_ENABLE   0x45414552 /* Q E A E */
#define VMW_ABSPOINTER_RELATIVE 0xF5
#define VMW_ABSPOINTER_ABSOLUTE 0x53424152 /* R A B S */
#define VMW_ABSPOINTER_QEMU     0x3442554a

#define VMW_ABSPOINTER_BUTTON1 0x20
#define VMW_ABSPOINTER_BUTTON2 0x10
#define VMW_ABSPOINTER_BUTTON3 0x08

#define VMW_CMD_GETVERSION 0x0a

class VMWare {
public:
	struct Command {
		union {
			uint32_t ax;
			uint32_t magic;
		};
		union {
			uint32_t bx;
			size_t size;
		};
		union {
			uint32_t cx;
			uint16_t command;
		};
		union {
			uint32_t dx;
			uint16_t port;
		};
		uint32_t si;
		uint32_t di;
	};

	static bool detect();
	static VMWare& inst();

	void enable_absolute_mouse();
	void disable_absolute_mouse();
	MouseEvent read_mouse_event();
	size_t mouse_queue_size();

	[[nodiscard]] bool present() const { return m_present; };
	[[nodiscard]] bool using_absolute_mouse() const { return m_absolute_mouse; };

private:
	void do_detect();

	void send(Command& cmd);
	void send_hb(Command& cmd);
	void get_hb(Command& cmd);

	static VMWare* s_inst;
	bool m_present = false;
	bool m_absolute_mouse = false;
};
