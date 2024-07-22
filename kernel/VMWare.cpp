/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "VMWare.h"
#include "kstd/KLog.h"

VMWare* VMWare::s_inst = nullptr;

bool VMWare::detect() {
	ASSERT(!s_inst);
	s_inst = new VMWare();
	s_inst->do_detect();
	return s_inst->m_present;
}

VMWare& VMWare::inst() {
	ASSERT(s_inst);
	return *s_inst;
}

void VMWare::do_detect() {
	Command cmd;
	cmd.bx = ~VMW_MAGIC;
	cmd.command = VMW_CMD_GETVERSION;
	send(cmd);
	m_present = cmd.bx == VMW_MAGIC && cmd.ax != 0xFFFFFFFF;
	if(m_present)
		KLog::dbg("VMWare", "Detected");
}

void VMWare::enable_absolute_mouse() {
	ASSERT(m_present);
	KLog::dbg("VMWare", "Enabling absolute mouse");
	Command cmd;
	cmd.bx = VMW_ABSPOINTER_ENABLE;
	cmd.command = VMW_ABSPOINTER_COMMAND;
	send(cmd);
	cmd.bx = 1;
	cmd.command = VMW_ABSPOINTER_DATA;
	send(cmd);
	if(cmd.ax != VMW_ABSPOINTER_QEMU) {
		KLog::dbg("VMWare", "Could not enable absolute mouse: Incorrect ID");
		return;
	}

	cmd.bx = 0;
	cmd.command = VMW_ABSPOINTER_STATUS;
	send(cmd);
	if(cmd.ax == VMW_ABSPOINTER_ERR) {
		KLog::dbg("VMWare", "Could not enable absolute mouse: Bad status");
		return;
	}

	cmd.bx = VMW_ABSPOINTER_ABSOLUTE;
	cmd.command = VMW_ABSPOINTER_COMMAND;
	send(cmd);

	m_absolute_mouse = true;
}

void VMWare::disable_absolute_mouse() {
	ASSERT(m_present);
	KLog::dbg("VMWare", "Disabling absolute mouse");
	Command cmd;
	cmd.bx = VMW_ABSPOINTER_RELATIVE;
	cmd.command = VMW_ABSPOINTER_COMMAND;
	send(cmd);
	m_absolute_mouse = false;
}

MouseEvent VMWare::read_mouse_event() {
	ASSERT(m_absolute_mouse);

	Command cmd;
	cmd.bx = 4;
	cmd.command = VMW_ABSPOINTER_DATA;
	send(cmd);

	MouseEvent event;
	event.x = cmd.bx;
	event.y = cmd.cx;
	event.z = (int8_t) cmd.dx;
	event.buttons = 0;
	if(cmd.ax & VMW_ABSPOINTER_BUTTON1)
		event.buttons |= 0x1;
	if(cmd.ax & VMW_ABSPOINTER_BUTTON2)
		event.buttons |= 0x2;
	if(cmd.ax & VMW_ABSPOINTER_BUTTON3)
		event.buttons |= 0x4;
	event.absolute = true;

	return event;
}

size_t VMWare::mouse_queue_size() {
	ASSERT(m_absolute_mouse);

	Command cmd;
	cmd.bx = 0;
	cmd.command = VMW_ABSPOINTER_STATUS;
	send(cmd);

	if(cmd.ax == 0xFFFF0000) {
		// Something happened, we'll restart it
		disable_absolute_mouse();
		enable_absolute_mouse();
	}

	return cmd.ax & 0xFFFF;
}

void VMWare::send(VMWare::Command& cmd) {
#if defined(__i386__)
	cmd.magic = VMW_MAGIC;
	cmd.port = VMW_PORT;
	asm volatile("in %%dx, %0" : "+a"(cmd.ax), "+b"(cmd.bx), "+c"(cmd.cx), "+d"(cmd.dx), "+S"(cmd.si), "+D"(cmd.di));
#endif
	//TODO: aarch64
}

void VMWare::send_hb(VMWare::Command& cmd) {
#if defined(__i386__)
	cmd.magic = VMW_MAGIC;
	cmd.port = VMW_PORTHB;
	asm volatile("cld; rep; outsb" : "+a"(cmd.ax), "+b"(cmd.bx), "+c"(cmd.cx), "+d"(cmd.dx), "+S"(cmd.si), "+D"(cmd.di));
#endif
	//TODO: aarch64
}

void VMWare::get_hb(VMWare::Command& cmd) {
#if defined(__i386__)
	cmd.magic = VMW_MAGIC;
	cmd.port = VMW_PORTHB;
	asm volatile("cld; rep; insb" : "+a"(cmd.ax), "+b"(cmd.bx), "+c"(cmd.cx), "+d"(cmd.dx), "+S"(cmd.si), "+D"(cmd.di));
#endif
	//TODO: aarch64
}