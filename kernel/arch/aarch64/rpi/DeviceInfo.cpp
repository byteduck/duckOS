/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "DeviceInfo.h"
#include "Mailbox.h"

#define MAILBOX_MEMORY_QUERY 0x10005
#define MAILBOX

using namespace RPi;

DeviceInfo* DeviceInfo::s_inst = nullptr;

DeviceInfo::DeviceInfo() {
	// TODO: Query memory ranges
}

DeviceInfo& DeviceInfo::inst() {
	if (__builtin_expect(!s_inst, false))
		s_inst = new DeviceInfo();
	return *s_inst;
}

size_t DeviceInfo::sdram_start() const {
	return 0;
}

size_t DeviceInfo::sdram_size() const {
	return 0x20000000; // TODO: Actually query this
}

size_t DeviceInfo::mmio_start() const {
	// TODO: Other models
	return 0x3F000000;
}

size_t DeviceInfo::mmio_size() const {
	return 0x01000000;
}

DeviceInfo::Model DeviceInfo::model() const {
	return DeviceInfo::RPi3b;
}
