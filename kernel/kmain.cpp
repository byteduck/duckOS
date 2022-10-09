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

#include "kmain.h"
#include "kernel/device/AC97Device.h"
#include <kernel/kstd/kstdio.h>
#include <kernel/memory/MemoryManager.h>
#include <kernel/memory/gdt.h>
#include <kernel/device/Device.h>
#include <kernel/time/TimeManager.h>
#include <kernel/interrupt/interrupt.h>
#include <kernel/CommandLine.h>
#include <kernel/device/BochsVGADevice.h>
#include <kernel/device/MultibootVGADevice.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/tasking/Process.h>
#include <kernel/tasking/Thread.h>
#include <kernel/device/PATADevice.h>
#include <kernel/terminal/VirtualTTY.h>
#include <kernel/filesystem/ext2/Ext2Filesystem.h>
#include <kernel/device/PartitionDevice.h>
#include <kernel/filesystem/FileDescriptor.h>
#include <kernel/User.h>
#include <kernel/filesystem/procfs/ProcFS.h>
#include <kernel/filesystem/VFS.h>
#include <kernel/filesystem/ptyfs/PTYFS.h>
#include <kernel/filesystem/socketfs/SocketFS.h>
#include <kernel/KernelMapper.h>
#include <kernel/tasking/ProcessArgs.h>
#include <kernel/kstd/KLog.h>
#include <kernel/tests/KernelTest.h>

uint8_t boot_disk;

typedef void (*constructor_func)();
extern constructor_func start_ctors[];
extern constructor_func end_ctors[];

//This method should be called with global constructors, so we'll assert that did_constructors == true after we do that
bool did_constructors = false;
__attribute__((constructor)) void constructor_test() {
	did_constructors = true;
}

//Use a uint8_t array to store the memory manager, or else it would be re-initialized when global constructors are called
uint8_t __mem_manager_storage[sizeof(MemoryManager)] __attribute__((aligned(4096)));

int kmain(uint32_t mbootptr){
	clearScreen();
	KLog::info("kinit", "Starting duckOS...");

	new (__mem_manager_storage) MemoryManager;

	struct multiboot_info mboot_header = parse_mboot(mbootptr);
	Memory::load_gdt();
	Interrupt::init();
	MemoryManager::inst().setup_paging();

	//Call global constructors, now that memory management is initialized
	for (constructor_func* ctor = start_ctors; ctor < end_ctors; ctor++)
		(*ctor)();
	ASSERT(did_constructors);

	TimeManager::init();
	Device::init();
	CommandLine cmd_line(mboot_header);

	//Try setting up VGA
	BochsVGADevice* bochs_vga = BochsVGADevice::create();
	if(!bochs_vga) {
		//We didn't find a bochs VGA device, try using the multiboot VGA device
		auto* mboot_vga = MultibootVGADevice::create(&mboot_header);
		if(!mboot_vga || mboot_vga->is_textmode())
			PANIC("MBOOT_TEXTMODE", "duckOS doesn't support textmode.");
	}

	clearScreen();
#ifdef DEBUG
	KLog::info("kinit", "Debug mode is enabled.");
#endif
	KLog::dbg("kinit", "First stage complete.");
	
	TaskManager::init();
	ASSERT(false); //We should never get here
	return 0;
}

void kmain_late(){
	KLog::dbg("kinit", "Tasking initialized.");

	auto* tty0 = new VirtualTTY(4, 0);
	tty0->set_active();
	setup_tty();

	KLog::dbg("kinit", "Initializing disk...");

	//Setup the disk (Assumes we're using primary master drive
	auto disk = kstd::shared_ptr<PATADevice>(PATADevice::find(
					PATADevice::PRIMARY,
					PATADevice::MASTER,
					CommandLine::inst().has_option("use_pio") //Use PIO if the command line option is present
				));
	if(!disk) {
		KLog::crit("kinit", "Couldn't find IDE controller! Hanging...");
		while(1);
	}

	//Find the LBA of the first partition
	auto* mbr_buf = new uint8_t[512];
	disk->read_block(0, mbr_buf);
	uint32_t part_offset = *((uint32_t*) &mbr_buf[0x1C6]);
	delete[] mbr_buf;

	//Set up the PartitionDevice with that LBA
	auto part = kstd::make_shared<PartitionDevice>(3, 1, disk, part_offset);
	auto part_descriptor = kstd::make_shared<FileDescriptor>(part);
	part_descriptor->set_options(O_RDWR);

	//Check if the filesystem is ext2
	if(!Ext2Filesystem::probe(*part_descriptor)) {
		KLog::crit("kinit", "Partition is not ext2! Hanging.");
		while(true);
	}

	//Setup the filesystem
	auto* ext2fs = new Ext2Filesystem(part_descriptor);
	ext2fs->init();
	if(ext2fs->superblock.version_major < 1){
		KLog::crit("kinit", "Unsupported ext2 version %d.%d. Must be at least 1. Hanging.", ext2fs->superblock.version_major, ext2fs->superblock.version_minor);
		while(true);
	}

	KLog::dbg("kinit", "Partition is ext2 %d.%d", ext2fs->superblock.version_major, ext2fs->superblock.version_minor);

	if(ext2fs->superblock.inode_size != 128){
		KLog::crit("kinit", "Unsupported inode size %d. DuckOS only supports an inode size of 128 at this time. Hanging.", ext2fs->superblock.inode_size);
		while(1);
	}

	//Setup the virtual filesystem and mount the ext2 filesystem as root
	auto* vfs = new VFS();
	if(!vfs->mount_root(ext2fs)) {
		KLog::crit("kinit", "Failed to mount root. Hanging.");
		while(true);
	}

	//Mount ProcFS
	auto root_user = User::root();
	auto proc_or_err = VFS::inst().resolve_path("/proc", VFS::inst().root_ref(), root_user);
	if(proc_or_err.is_error()) {
		KLog::crit("kinit", "Failed to mount proc: %d", proc_or_err.code());
		while(true);
	}

	auto* procfs = new ProcFS();
	auto res = VFS::inst().mount(procfs, proc_or_err.value());
	if(res.is_error()) {
		KLog::crit("kinit", "Failed to mount proc: %d", res.code());
		while(true);
	}

	//Mount SocketFS
	auto sock_or_err = VFS::inst().resolve_path("/sock", VFS::inst().root_ref(), root_user);
	if(sock_or_err.is_error()) {
		KLog::crit("kinit", "Failed to mount sock: %d", sock_or_err.code());
		while(true);
	}

	auto* socketfs = new SocketFS();
	res = VFS::inst().mount(socketfs, sock_or_err.value());
	if(res.is_error()) {
		KLog::crit("kinit", "Failed to mount sock: %d", res.code());
		while(true);
	}

	//Mount PTYFS
	auto pts_or_err = VFS::inst().resolve_path("/dev/pts", VFS::inst().root_ref(), root_user);
	if(pts_or_err.is_error()) {
		KLog::crit("kinit", "Failed to mount pts: %d", pts_or_err.code());
		while(true);
	}

	auto* ptyfs = new PTYFS();
	res = VFS::inst().mount(ptyfs, pts_or_err.value());
	if(res.is_error()) {
		KLog::crit("kinit", "Failed to mount pts: %d", res.code());
		while(true);
	}

	//Load the kernel symbols
	KernelMapper::load_map();

	//Try initializing the sound card
	auto dev = AC97Device::detect();

	//If we're running tests, do so
	if(CommandLine::inst().get_option_value("kernel-tests") == "true") {
		KernelTestRegistry::inst().run_tests();
		while(true);
	}

	kstd::string launch_command;

	if(CommandLine::inst().get_option_value("boot-terminal") == "true") {
		KLog::dbg("kinit", "Starting dsh...");
		launch_command = "/bin/dsh";
	} else {
		KLog::dbg("kinit", "Starting init...");
		launch_command = "/bin/init";
	}

	//Replace kinit with init
	auto* init_args = new ProcessArgs(VFS::inst().root_ref());
	init_args->argv.push_back(launch_command);
	TaskManager::current_thread()->process()->exec(launch_command, init_args);

	//We shouldn't get here
	PANIC("INIT_FAILED", "Failed to start init.");
	ASSERT(false);
}

struct multiboot_info parse_mboot(uint32_t physaddr){
	auto* header = (struct multiboot_info*) (physaddr + HIGHER_HALF);

	//Check boot disk
	if(header->flags & MULTIBOOT_INFO_BOOTDEV) {
		boot_disk = (header->boot_device & 0xF0000000u) >> 28u;
		KLog::dbg("kinit", "BIOS boot disk: 0x%x", boot_disk);
	} else {
		PANIC("MULTIBOOT_FAIL", "The multiboot header doesn't have boot device info. Cannot boot.");
	}

	//Parse memory map
	if(header->flags & MULTIBOOT_INFO_MEM_MAP) {
		auto* mmap_entry = (multiboot_mmap_entry*) (header->mmap_addr + HIGHER_HALF);
		MemoryManager::inst().parse_mboot_memory_map(header, mmap_entry);
	} else {
		PANIC("MULTIBOOT_FAIL", "The multiboot header doesn't have a memory map. Cannot boot.");
	}

	return *header;
}
