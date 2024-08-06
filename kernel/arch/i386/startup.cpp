/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include <kernel/kstd/types.h>
#include <kernel/memory/MemoryManager.h>
#include <kernel/multiboot.h>
#include <kernel/CommandLine.h>
#include <kernel/kstd/KLog.h>
#include <kernel/constructors.h>

multiboot_info mboot_header;
multiboot_mmap_entry* mmap_entry;

struct multiboot_info parse_mboot(size_t physaddr);
extern uint8_t boot_disk;

extern "C" void kmain();

extern "C" void i386init(uint32_t mbootptr) {
	call_global_constructors();
	mboot_header = parse_mboot(mbootptr);
	CommandLine cmd_line(mboot_header);
	kmain();
	ASSERT(false);
}

struct multiboot_info parse_mboot(size_t physaddr){
	auto* header = (struct multiboot_info*) (physaddr + HIGHER_HALF);

	//Check boot disk
	if(header->flags & MULTIBOOT_INFO_BOOTDEV) {
		boot_disk = (header->boot_device & 0xF0000000u) >> 28u;
		KLog::dbg("kinit", "BIOS boot disk: {#x}", boot_disk);
	} else {
		PANIC("MULTIBOOT_FAIL", "The multiboot header doesn't have boot device info. Cannot boot.");
	}

	//Parse memory map
	if(header->flags & MULTIBOOT_INFO_MEM_MAP) {
		mmap_entry = (multiboot_mmap_entry*) ((size_t) header->mmap_addr + HIGHER_HALF);
	} else {
		PANIC("MULTIBOOT_FAIL", "The multiboot header doesn't have a memory map. Cannot boot.");
	}

	return *header;
}