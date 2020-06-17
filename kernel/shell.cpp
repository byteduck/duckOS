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

    Copyright (c) Byteduck 2016-$YEAR. All rights reserved.
*/

#include <kernel/kstddef.h>
#include <kernel/keyboard.h>
#include <kernel/kstdio.h>
#include <kernel/shell.h>
#include <kernel/filesystem/Ext2.h>
#include <kernel/memory/kliballoc.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/tasking/elf.h>
#include <kernel/pci/pci.h>
#include <kernel/memory/paging.h>
#include <kernel/device/ide.h>
#include <kernel/filesystem/VFS.h>
#include <common/cstring.h>
#include <common/stdlib.h>
#include <common/defines.h>
#include <kernel/filesystem/InodeFile.h>
#include <kernel/device/TTYDevice.h>

char kbdbuf[512];
extern bool tasking_enabled;

Shell::Shell(): current_dir(VFS::inst().root_ref()){
}

void dummy(){
	while(1);
}

void Shell::shell(){
	printf(
		"O--------------------O\n"
		"| Welcome to duckOS! |\n"
		"O--------------------O\n"
	);
	auto tty_or_error = VFS::inst().open("/dev/tty0", O_RDONLY, 0, VFS::inst().root_ref());
	if(tty_or_error.is_error()) {
		printf("Could not open tty0: %d\n", tty_or_error.code());
		return;
	}
	auto tty = tty_or_error.value();
	while(!exitShell){
		printf("kernel:%s$ ", current_dir->get_full_path().c_str());
		ssize_t nread;
		while(!(nread = tty->read((uint8_t*)kbdbuf, 511)));
		kbdbuf[nread - 1] = '\0';
		setColor(0x07);
		substr(indexOf(' ', kbdbuf), kbdbuf, cmdbuf);
		if(indexOf(' ', kbdbuf)+1 <= strlen(kbdbuf)){
			substrr(indexOf(' ', kbdbuf)+1, strlen(kbdbuf), kbdbuf, argbuf);
		}else{
			argbuf[0] = 0;
		}
		command_eval(cmdbuf, argbuf);
		setColor(0x0f);
	}
	TaskManager::current_process()->kill();
}

void Shell::command_eval(char *cmd, char *args){
	if(strcmp(cmd,"help")){
		println("ls: List the files in the current directory. Use -h for help.");
		println("cd: Change the current directory.");
		println("pwd: Print the working directory.");
		println("about: Shows some information about the system.");
		println("help: Shows this message.");
		println("cat: Prints a file's contents.");
		println("about: Prints some information.");
		println("mem: Prints information about the memory.");
		println("pagefault: Triggers a page fault, in case you wanted to.");
		println("tasks: Prints all running tasks.");
		println("bg: Run a program in the background.");
		println("kill: Kill a program.");
		println("dummy: Create a dummy process.");
		println("readelf: Print info about an ELF executable.");
		println("lspci: Lists PCI devices.");
		println("exit: Pretty self explanatory.");
	}else if(strcmp(cmd,"ls")){
		DC::string path = ".";
		if(!strcmp(args, "")) path = args;
		auto desc_ret = VFS::inst().open(path, O_RDONLY | O_DIRECTORY, MODE_DIRECTORY, current_dir);
		if(desc_ret.is_error()) {
			switch(desc_ret.code()) {
				case -ENOTDIR:
					printf("Couldn't ls: '%s' is not a directory\n", args);
					break;
				case -ENOENT:
					printf("Couldn't ls: '%s' does not exist\n", args);
					break;
				default:
					printf("Couldn't ls: %d\n", desc_ret.code());
			}
		} else {
			auto desc = desc_ret.value();
			DirectoryEntry buffer {};
			while(desc->read_dir_entry(&buffer)) {
				switch(buffer.type) {
					case TYPE_UNKNOWN:
						printf("[?]    ");
						break;
					case TYPE_FILE:
						printf("[file] ");
						break;
					case TYPE_DIR:
						printf("[dir]  ");
						break;
					case TYPE_CHARACTER_DEVICE:
					case TYPE_BLOCK_DEVICE:
					case TYPE_FIFO:
					case TYPE_SOCKET:
						printf("[dev]  ");
						break;
					case TYPE_SYMLINK:
						printf("[link] ");
						break;
				}
				printf("%d %s\n", buffer.id, buffer.name);
			}
		}
	}else if(strcmp(cmd,"cd")){
		auto ref = VFS::inst().resolve_path(args, current_dir, nullptr);
		if(!ref.is_error()) {
			auto val = ref.value();
			if(val->inode()->metadata().is_directory())
				current_dir = val;
			else
				printf("Could not cd to '%s': Not a directory\n", args);
		} else {
			printf("Could not find '%s'\n", args);
		}
	}else if(strcmp(cmd,"pwd")){
		printf("%s\n", current_dir->get_full_path().c_str());
	}else if(strcmp(cmd,"about")){
		println("DuckOS v0.1");
	}else if(strcmp(cmd, "mem")) {
		printf("Used memory: %dKiB\n", Paging::get_used_mem());
	}else if(strcmp(cmd,"cat")){
		auto desc_ret = VFS::inst().open(args, O_RDONLY, MODE_FILE, current_dir);
		if(desc_ret.is_error()) {
			switch (desc_ret.code()) {
				case -ENOENT:
					printf("Cannot cat '%s': no such file\n", args);
					break;
				case -ENOTDIR:
					printf("Cannot cat '%s': path is not a directory\n", args);
					break;
				case -ENODEV:
					printf("Cannot cat '%s': no such device\n", args);
					break;
				default:
					printf("Cannot cat '%s': Error %d\n", args, desc_ret.code());
			}
		} else {
			auto desc = desc_ret.value();
			if(desc->metadata().is_directory()) printf("Cannot cat '%s': is a directory\n", args);
			else {
				auto* buf = new uint8_t[513];
				size_t nread;
				while((nread = desc->read(buf, 512))) {
					buf[nread] = '\0';
					printf((char*)buf);
				}
				delete[] buf;
			}
		}
	}else if(strcmp(cmd,"pagefault")){
		if(strcmp(args,"-r")){
			char i = ((char*)0xDEADC0DE)[0];
		}else if(strcmp(args,"-w")){
			((char*)0xDEADC0DE)[0]='F';
		}else{
			println("Usage: pagefault [-r,-w]");
			println("-r: Triggers a page fault by reading.");
			println("-w: Triggers a page fault by writing.");
		}
	}else if(strcmp(cmd,"exit")){
		exitShell = true;
	}else if(strcmp(cmd,"tasks")){
		TaskManager::print_tasks();
	}else if(strcmp(cmd,"kill")){
		uint32_t pid = atoi(args);
		Process *proc = TaskManager::process_for_pid(pid);
		if(proc != NULL && proc->pid() != 1){
			TaskManager::kill(proc);
			printf("Sent SIGTERM (%d) to %s (PID %d).\n", SIGTERM, proc->name().c_str(), proc->pid());
		}else if(proc->pid() == 1)
			printf("Cannot kill kernel!\n");
		else
			printf("No process with PID %d.\n", pid);
	}else if(strcmp(cmd, "dummy")){
		TaskManager::add_process(Process::create_kernel("dummy", dummy));
	}else if(strcmp(cmd, "readelf")){
		auto desc_ret = VFS::inst().open(args, O_RDONLY, MODE_FILE, current_dir);
		if(desc_ret.is_error()) {
			switch (desc_ret.code()) {
				case -ENOENT:
					printf("Cannot cat '%s': no such file\n", args);
					break;
				case -ENOTDIR:
					printf("Cannot cat '%s': path is not a directory\n", args);
					break;
				case -ENODEV:
					printf("Cannot cat '%s': no such device\n", args);
					break;
				default:
					printf("Cannot cat '%s': Error %d\n", args, desc_ret.code());
			}
		} else {
			auto fd = desc_ret.value();
			auto* header = new ELF::elf32_header;
			fd->read((uint8_t*)header, sizeof(ELF::elf32_header));
			printf("Bits: %s\n", header->bits == ELF32 ? "32" : "64");
			printf("Endianness: %s\n", header->endianness == ELF_LITTLE_ENDIAN ? "little" : "big");
			printf("Instruction set: %s\n", header->instruction_set == ELF_X86 ? "x86" : "not x86");
			printf("Version: 0x%x\n", header->elf_version);
			printf("(Can%s execute, entry=0x%x)\n", ELF::can_execute(header) ? "" : "'t", header->program_entry_position);

			uint32_t pheader_loc = header->program_header_table_position;
			uint32_t pheader_size = header->program_header_table_entry_size;
			uint32_t num_pheaders = header->program_header_table_entries;

			fd->seek(pheader_loc, SEEK_SET);
			auto* program_headers = new ELF::elf32_segment_header[num_pheaders];
			fd->read((uint8_t*)program_headers, pheader_size * num_pheaders);

			for(auto i = 0; i < num_pheaders; i++) {
				ELF::elf32_segment_header* header = &program_headers[i];
				printf("--Section--\n");
				printf("Type: ");
				switch(header->p_type) {
					case ELF_PT_NULL:
						printf("PT_NULL");
						break;
					case ELF_PT_LOAD:
						printf("PT_LOAD");
						break;
					case ELF_PT_DYNAMIC:
						printf("PT_DYNAMIC");
						break;
					case ELF_PT_INTERP:
						printf("PT_INTERP");
						break;
					case ELF_PT_NOTE:
						printf("PT_NOTE");
						break;
					case ELF_PT_SHLIB:
						printf("PT_SHLIB");
						break;
					case ELF_PT_PHDR:
						printf("PT_PHDR");
						break;
					default:
						printf("PT_LOPROC/PT_HIPROC");
				}
				printf("\nOffset: 0x%x, ", header->p_offset);
				printf("Vaddr: 0x%x, ", header->p_vaddr);
				printf("Filesz: 0x%x, ", header->p_filesz);
				printf("Memsz: 0x%x\n", header->p_memsz);
				DC::string flags;
				if(header->p_flags & ELF_PF_R) flags += "R";
				if(header->p_flags & ELF_PF_W) flags += "W";
				if(header->p_flags & ELF_PF_X) flags += "X";
				printf("Flags: %s, ", flags.c_str());
				printf("Align: 0x%x\n", program_headers[i].p_align);
			}

			delete[] program_headers;
			delete header;
		}
	}else if(strcmp(cmd, "lspci")){
		PCI::enumerate_devices([](PCI::Address address, PCI::ID id, void* dataPtr) {
			uint8_t clss = PCI::read_byte(address, PCI_CLASS);
			uint8_t subclss = PCI::read_byte(address, PCI_SUBCLASS);
			printf("%x:%x.%x Vendor: 0x%x Device: 0x%x\n  Class: 0x%x Subclass: 0x%x\n", address.bus, address.slot, address.function, id.vendor, id.device, clss, subclss);
		}, nullptr);
	}else if(strcmp(cmd, "findpata")){
		IDE::PATAChannel channel = IDE::find_pata_channel(ATA_PRIMARY);
		printf("%x:%x.%x Int %d\n", channel.address.bus, channel.address.slot, channel.address.function, PCI::read_byte(channel.address, PCI_INTERRUPT_LINE));
	}else{
		DC::string cmds = cmd;
		ResultRet<Process *> p(0);
		if(cmds.find("/") != -1)
			p = Process::create_user(cmd);
		else
			p = Process::create_user(DC::string("/bin/") + cmds);
		if(p.is_error()) {
			if(p.code() == -ENOENT) printf("Could not find command '%s'.\n", cmd);
			else printf("Error creating process: %d\n", p.code());
			return;
		}
		TaskManager::add_process(p.value());
		pid_t pid = p.value()->pid();
		while(TaskManager::process_for_pid(pid));
	}
}
