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

#ifndef DUCKOS_PROCESS_H
#define DUCKOS_PROCESS_H

#include <kernel/kstddef.h>
#include <common/string.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/tasking/elf.h>
#include <common/shared_ptr.hpp>

#define PROCESS_STACK_SIZE 4096

class Process {
public:
	~Process();

	static Process* create_kernel(const DC::string& name, void (*func)());
	static ResultRet<Process*> create_user(const DC::string& executable_loc);

	pid_t pid();
	DC::string name();
	void notify(uint32_t sig);
	void kill();
	void init();

	uint32_t state = 0;
	Paging::PageDirectory* page_directory;
	bool inited = false;
	Process *next = nullptr, *prev = nullptr;
	size_t page_directory_loc;
	Registers registers = {};
	bool kernel = false;

	DC::shared_ptr<FileDescriptor> stdin;
	DC::shared_ptr<FileDescriptor> stdout;

	//Syscalls
	ssize_t sys_read(int fd, uint8_t* buf, size_t count);
	ssize_t sys_write(int fd, uint8_t* buf, size_t count);
	size_t sys_sbrk(int i);

private:
	Process(const DC::string& name, size_t entry_point, bool kernel = false);

	bool load_elf(const DC::shared_ptr<FileDescriptor>& fd, ELF::elf32_header* header);

	DC::string _name = "";
	pid_t _pid = 0;
	size_t current_brk = 0;
};


#endif //DUCKOS_PROCESS_H
