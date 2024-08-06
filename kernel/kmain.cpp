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
#include "VMWare.h"
#include <kernel/kstd/kstdio.h>
#include <kernel/memory/MemoryManager.h>
#include <kernel/device/Device.h>
#include <kernel/time/TimeManager.h>
#include <kernel/interrupt/interrupt.h>
#include <kernel/CommandLine.h>
#include <kernel/device/MultibootVGADevice.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/tasking/Process.h>
#include <kernel/tasking/Thread.h>
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
#include "bootlogo.h"
#include "arch/Processor.h"
#include "net/NetworkAdapter.h"

#if defined(__i386__)
#include "arch/i386/device/PATADevice.h"
#endif

uint8_t boot_disk;

void kmain(){
	KLog::info("kinit", "Starting duckOS...");

	Processor::init();
	Memory::init();
	Processor::init_interrupts();
	VMWare::detect();
	Device::init();

	// Clear screen and draw boot logo
	clearScreen();
	size_t logo_pos[2] = {
		VGADevice::inst().get_display_width() / 2 - (BOOT_LOGO_WIDTH * BOOT_LOGO_SCALE) / 2,
		VGADevice::inst().get_display_height() / 2 - (BOOT_LOGO_HEIGHT * BOOT_LOGO_SCALE) / 2
	};
	for(size_t y = 0; y < BOOT_LOGO_HEIGHT * BOOT_LOGO_SCALE; y++) {
		for(size_t x = 0; x < BOOT_LOGO_WIDTH * BOOT_LOGO_SCALE; x++) {
			VGADevice::inst().set_pixel(logo_pos[0] + x, logo_pos[1] + y, boot_logo[(x / BOOT_LOGO_SCALE) + (y / BOOT_LOGO_SCALE) * BOOT_LOGO_WIDTH]);
		}
	}

	auto* tty0 = new VirtualTTY(4, 0);
	tty0->set_active();
	setup_tty();

#ifdef DEBUG
	KLog::info("kinit", "Debug mode is enabled.");
#endif
	KLog::dbg("kinit", "First stage complete.");
	
	TaskManager::init();

	ASSERT(false); //We should never get here
}

void kmain_late(){
	KLog::dbg("kinit", "Tasking initialized.");

	TimeManager::init();


#if defined(__aarch64__)
	KLog::dbg("kinit", "TODO aarch64");
	while (1);
	// TODO: aarch64
#elif defined(__i386__)
	KLog::dbg("kinit", "Initializing disk...");

	//Setup the disk (Assumes we're using primary master drive
	auto disk = kstd::Arc<PATADevice>(PATADevice::find(
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
		KLog::crit("kinit", "Unsupported ext2 version {}.{}. Must be at least 1. Hanging.",
					ext2fs->superblock.version_major, ext2fs->superblock.version_minor);
		while(true);
	}

	KLog::dbg("kinit", "Partition is ext2 {}.{}", ext2fs->superblock.version_major, ext2fs->superblock.version_minor);

	if(ext2fs->superblock.inode_size != 128){
		KLog::crit("kinit",
					"Unsupported inode size {}. DuckOS only supports an inode size of 128 at this time. Hanging.",
					ext2fs->superblock.inode_size);
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
		KLog::crit("kinit", "Failed to mount proc: {}", proc_or_err.code());
		while(true);
	}

	auto* procfs = new ProcFS();
	auto res = VFS::inst().mount(procfs, proc_or_err.value());
	if(res.is_error()) {
		KLog::crit("kinit", "Failed to mount proc: {}", res.code());
		while(true);
	}

	//Mount SocketFS
	auto sock_or_err = VFS::inst().resolve_path("/sock", VFS::inst().root_ref(), root_user);
	if(sock_or_err.is_error()) {
		KLog::crit("kinit", "Failed to mount sock: {}", sock_or_err.code());
		while(true);
	}

	auto* socketfs = new SocketFS();
	res = VFS::inst().mount(socketfs, sock_or_err.value());
	if(res.is_error()) {
		KLog::crit("kinit", "Failed to mount sock: {}", res.code());
		while(true);
	}

	//Mount PTYFS
	auto pts_or_err = VFS::inst().resolve_path("/dev/pts", VFS::inst().root_ref(), root_user);
	if(pts_or_err.is_error()) {
		KLog::crit("kinit", "Failed to mount pts: {}", pts_or_err.code());
		while(true);
	}

	auto* ptyfs = new PTYFS();
	res = VFS::inst().mount(ptyfs, pts_or_err.value());
	if(res.is_error()) {
		KLog::crit("kinit", "Failed to mount pts: {}", res.code());
		while(true);
	}

	//Load the kernel symbols
	KernelMapper::load_map();

	//Try initializing network
	NetworkAdapter::setup();

	//If we're running tests, do so
	if(CommandLine::inst().get_option_value("kernel-tests") == "true") {
		KernelTestRegistry::inst().run_tests();
		while(true);
	}

	KLog::dbg("kinit", "Starting init...");

	//Replace kinit with init
	auto* init_args = new ProcessArgs(VFS::inst().root_ref());
	init_args->argv.push_back("/bin/init");
	TaskManager::current_thread()->process()->exec(kstd::string("/bin/init"), init_args);

	//We shouldn't get here
	PANIC("INIT_FAILED", "Failed to start init.");
	ASSERT(false);
#endif
}
