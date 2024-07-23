/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Mailbox.h"
#include <kernel/kstd/kstdio.h>

using namespace RPi;

bool Mailbox::call(void* buffer, Channel channel) {
	if ((size_t) buffer & 0xF || (size_t) buffer > 0xFFFFFFFF)
		return false; // Not properly aligned

	auto header = ((uint32_t) (size_t) buffer) | (channel & 0xF);
	// TODO: Flush cache memory?

	while (get<uint32_t>(STATUS) & FULL) {}
	set<uint32_t>(WRITE, header);

	while (true) {
		while (get<uint32_t>(STATUS) & EMPTY) {}
		if (header == get<uint32_t>(READ))
			return ((uint32_t*) buffer)[1] == RESPONSE;
	}
}