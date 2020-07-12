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

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <kernel/kstddef.h>
#include <kernel/multiboot.h>
#include <kernel/kstdio.h>
#include <kernel/memory/memory.h>
#include <kernel/interrupt/idt.h>
#include <kernel/interrupt/isr.h>
#include <kernel/interrupt/irq.h>
#include <kernel/filesystem/Ext2.h>
#include <kernel/shell.h>
#include <kernel/pit.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/device/PIODevice.h>
#include <kernel/device/PartitionDevice.h>
#include <kernel/kmain.h>
#include <kernel/filesystem/VFS.h>
#include <kernel/device/TTYDevice.h>
#include <kernel/device/KeyboardDevice.h>
#include <common/defines.h>
#include <kernel/device/BochsVGADevice.h>
#include <kernel/device/MultibootVGADevice.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/device/PATADevice.h>

uint8_t boot_disk;

int kmain(uint32_t mbootptr){
	clearScreen();
	printf("init: Starting duckOS...\n");
	struct multiboot_info mboot_header = parse_mboot(mbootptr);
	load_gdt();
	interrupts_init();
	Memory::setup_paging();
	Device::init();

	//Try setting up VGA
	BochsVGADevice* bochs_vga = BochsVGADevice::create();
	if(bochs_vga) {
		set_graphical_mode(bochs_vga->get_framebuffer_width(), bochs_vga->get_framebuffer_height(), bochs_vga->get_framebuffer());
	} else {
		auto* mboot_vga = MultibootVGADevice::create(&mboot_header);
		if(mboot_vga) {
			if(mboot_vga->is_textmode()) {
				set_text_mode(mboot_vga->get_framebuffer_width(), mboot_vga->get_framebuffer_height(), mboot_vga->get_framebuffer());
			} else {
				set_graphical_mode(mboot_vga->get_framebuffer_width(), mboot_vga->get_framebuffer_height(), mboot_vga->get_framebuffer());
			}
		} else {
			printf("vga: Falling back to text mode.\n");
			void* vidmem = (void*) PageDirectory::k_mmap(0xB8000, 0xFA0, true);
			set_text_mode(80, 25, vidmem);
		}
	}

	clearScreen();

#ifdef DEBUG
	printf("init: Debug mode is enabled.\n");
#endif

	printf("init: First stage complete.\ninit: Initializing tasking...\n");

	TaskManager::init();

	return 0;
}

void shell_process(){
	int i = 0;
	Shell shell;
	shell.shell();
}

//called from ktask
void kmain_late(){
	new KeyboardDevice;

	printf("init: Tasking initialized.\ninit: Initializing TTY...\n");

	auto* tty0 = new TTYDevice(0, "tty0", 4, 0);
	tty0->set_active();

	printf("init: TTY initialized.\ninit: Initializing disk...\n");

	auto tmpdisk = DC::make_shared<PIODevice>(99, 99, 8);
	//auto disk = DC::shared_ptr<PATADevice>(PATADevice::find(PATADevice::PRIMARY, PATADevice::MASTER));
	auto part = DC::make_shared<PartitionDevice>(3, 1, tmpdisk, tmpdisk->get_first_partition());
	auto part_descriptor = DC::make_shared<FileDescriptor>(part);

	if(Ext2Filesystem::probe(*part_descriptor)){
		printf("init: Partition is ext2 ");
	}else{
		println("init: Partition is not ext2! Hanging.");
		while(true);
	}

	auto* ext2fs = new Ext2Filesystem(part_descriptor);
	ext2fs->init();
	if(ext2fs->superblock.version_major < 1){
		printf("init: Unsupported ext2 version %d.%d. Must be at least 1. Hanging.", ext2fs->superblock.version_major, ext2fs->superblock.version_minor);
		while(true);
	}
	printf("%d.%d\n", ext2fs->superblock.version_major, ext2fs->superblock.version_minor);
	if(ext2fs->superblock.inode_size != 128){
		printf("init: Unsupported inode size %d. DuckOS only supports an inode size of 128 at this time. Hanging.", ext2fs->superblock.inode_size);
	}

	VFS* vfs = new VFS();
	if(!vfs->mount_root(ext2fs)) {
		printf("init: Failed to mount root. Hanging.");
		while(true);
	}

	printf("init: Done!\n");

	pid_t shell_pid = TaskManager::add_process(Process::create_kernel("shell", shell_process));
}

struct multiboot_info parse_mboot(uint32_t physaddr){
	auto* header = (struct multiboot_info*) (physaddr + HIGHER_HALF);

	if(!header) PANIC("MULTIBOOT_FAIL", "Failed to k_mmap memory for the multiboot header.\n", true);

	//Check boot disk
	if(header->flags & MULTIBOOT_INFO_BOOTDEV) {
		boot_disk = (header->boot_device & 0xF0000000u) >> 28u;
		printf("init: BIOS boot disk: 0x%x\n", boot_disk);
	} else {
		PANIC("MULTIBOOT_FAIL", "The multiboot header doesn't have boot device info. Cannot boot.\n", true);
	}

	//Parse memory map
	//TODO: Actually keep track of the information and use it
	if(header->flags & MULTIBOOT_INFO_MEM_MAP) {
		auto* mmap_entry = (multiboot_mmap_entry*) (header->mmap_addr + HIGHER_HALF);
		Memory::parse_mboot_memory_map(header, mmap_entry);
	} else {
		PANIC("MULTIBOOT_FAIL", "The multiboot header doesn't have a memory map. Cannot boot.\n", true);
	}

	return *header;
}

void interrupts_init(){
	Interrupt::register_idt();
	Interrupt::isr_init();
	Interrupt::idt_set_gate(0x80, (unsigned)asm_syscall_handler, 0x08, 0xEF);
	PIT::init();
	Interrupt::irq_init();
	sti();
}
