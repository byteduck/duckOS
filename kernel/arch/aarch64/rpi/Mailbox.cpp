/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Mailbox.h"
#include <kernel/kstd/kstdio.h>
#include "MMIO.h"
#include <kernel/memory/MemoryManager.h>

using namespace RPi;

bool Mailbox::call(void* buffer, size_t size, Channel channel) {
	auto buf_phys = MM.kernel_page_directory.get_physaddr(buffer);

	if (buf_phys & 0xF || buf_phys > 0xFFFFFFFF)
		return false; // Not properly aligned

	// Wait until we can write
	while (MMIO::peek<uint32_t>(STATUS) & FULL) {}

	auto header = ((uint32_t) buf_phys) | (channel & 0xF);

	// Flush out cache before sending to make sure VideoCore memory is in sync
	Aarch64::flush_cache({(size_t) buffer, size});

	MMIO::poke<uint32_t>(WRITE, header);

	while (true) {
		while (MMIO::peek<uint32_t>(STATUS) & EMPTY) {}
		if (header == MMIO::peek<uint32_t>(READ))
			return ((uint32_t*) buffer)[1] == RESPONSE;
	}
}