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

#include <kernel/memory/kliballoc.h>
#include <kernel/filesystem/VFS.h>
#include <common/defines.h>
#include <kernel/device/TTYDevice.h>
#include "Process.h"
#include "TaskManager.h"
#include "elf.h"
#include "ProcessArgs.h"

Process* Process::create_kernel(const DC::string& name, void (*func)()){
	ProcessArgs args = ProcessArgs(DC::shared_ptr<LinkedInode>(nullptr));
	return new Process(name, (size_t)func, true, &args, 0);
}

ResultRet<Process*> Process::create_user(const DC::string& executable_loc, ProcessArgs* args, pid_t parent) {
	auto fd_or_error = VFS::inst().open(executable_loc, O_RDONLY, 0, VFS::inst().root_ref());
	if(fd_or_error.is_error()) return fd_or_error.code();

	auto fd = fd_or_error.value();
	auto* header = new ELF::elf32_header;
	fd->read((uint8_t*)header, sizeof(ELF::elf32_header));
	if(!ELF::can_execute(header)) {
		delete header;
		return -ENOEXEC;
	}

	TaskManager::enabled() = false;

	auto* proc = new Process(executable_loc, header->program_entry_position, false, args, parent);

	bool success = proc->load_elf(fd, header);

	//Load the page directory of the current process back
	asm volatile("movl %0, %%cr3" :: "r"(TaskManager::current_process()->page_directory_loc));

	TaskManager::enabled() = true;

	delete header;
	if(!success) {
		delete proc;
		return -ENOEXEC;
	}


	return proc;
}

bool Process::load_elf(const DC::shared_ptr<FileDescriptor> &fd, ELF::elf32_header* header) {
	//FIXME: Dealloc phys pages if creation fails

	uint32_t pheader_loc = header->program_header_table_position;
	uint32_t pheader_size = header->program_header_table_entry_size;
	uint32_t num_pheaders = header->program_header_table_entries;

	fd->seek(pheader_loc, SEEK_SET);
	auto* program_headers = new ELF::elf32_segment_header[num_pheaders];
	fd->read((uint8_t*)program_headers, pheader_size * num_pheaders);

	for(auto i = 0; i < num_pheaders; i++) {
		auto pheader = &program_headers[i];
		if(pheader->p_type == ELF_PT_LOAD) {
			size_t loadloc_pagealigned = (pheader->p_vaddr/PAGE_SIZE) * PAGE_SIZE;
			size_t loadsize_pagealigned = pheader->p_memsz + (pheader->p_vaddr % PAGE_SIZE);
			if(!page_directory->allocate_pages(loadloc_pagealigned, loadsize_pagealigned, pheader->p_flags & ELF_PF_W)) {
				delete[] program_headers;
				return false;
			}

			fd->seek(pheader->p_offset, SEEK_SET);
			fd->read((uint8_t*)pheader->p_vaddr, pheader->p_filesz);

			if(current_brk < pheader->p_vaddr + pheader->p_memsz)
				current_brk = pheader->p_vaddr + pheader->p_memsz;
		}
	}

	delete[] program_headers;
	return true;
}


pid_t Process::pid() {
	return _pid;
}

DC::string Process::name(){
	return _name;
}

//TODO: Clean up this monstrosity
Process::Process(const DC::string& name, size_t entry_point, bool kernel, ProcessArgs* args, pid_t parent) {
	_name = name;
	_pid = TaskManager::get_new_pid();
	state = PROCESS_ALIVE;
	this->kernel = kernel;
	ring = kernel ? 0 : 3;
	parent = parent;
	if(!kernel) {
		auto ttydesc = DC::make_shared<FileDescriptor>(TTYDevice::current_tty());
		file_descriptors.push_back(ttydesc);
		file_descriptors.push_back(ttydesc);
		cwd = args->working_dir;
	}

	_kernel_stack_base = Paging::PageDirectory::k_alloc_pages(PROCESS_KERNEL_STACK_SIZE);
	_kernel_stack_size = PROCESS_KERNEL_STACK_SIZE;

	size_t stack_base;
	if(!kernel) {
		page_directory = new Paging::PageDirectory();
		page_directory_loc = page_directory->entries_physaddr();

		//Load the page directory of the new process
		asm volatile("movl %0, %%cr3" :: "r"(page_directory_loc));

		if(!page_directory->allocate_pages(HIGHER_HALF - PROCESS_STACK_SIZE, PROCESS_STACK_SIZE))
			PANIC("NEW_PROC_STACK_ALLOC_FAIL", "Was unable to allocate virtual memory for a new process's stack.", true);
		stack_base = HIGHER_HALF - PROCESS_STACK_SIZE;
		_stack_size = PROCESS_STACK_SIZE;
	} else {
		page_directory_loc = Paging::kernel_page_directory.entries_physaddr();
		stack_base = (size_t) _kernel_stack_base;
		_stack_size = PROCESS_KERNEL_STACK_SIZE;
	}

	registers.eip = entry_point;

	auto *stack = (uint32_t*) (stack_base + _stack_size);

	stack = (uint32_t*) args->setup_stack(stack);

	*--stack = 0; //Honestly? Not sure what this is for but nothing works without it :)

	//If this is a usermode process, push the correct ss and stack pointer for switching rings during iret
	if(!kernel) {
		auto cstack = (size_t) stack;
		*--stack = 0x23;
		*--stack = cstack;
	}

	//Push EFLAGS, CS, and EIP for iret
	*--stack = 0x202; // eflags
	*--stack = kernel ? 0x8 : 0x1B; // cs
	*--stack = entry_point; // eip

	//We push iret if the pid isn't 1 (the kernel process), because in the preempt_asm function, we return to preempt in
	//the C code. However, when the process is first set up, the ret location isn't in the stack, so we push a function
	//that pops the initial values of the registers and calls iret.
	if(_pid != 1){
		*--stack = 0; //eax
		*--stack = 0; //ebx
		*--stack = 0; //ecx
		*--stack = 0; //edx
		*--stack = (size_t) TaskManager::proc_first_preempt;
	}

	//Push everything that's popped off in preempt_asm
	*--stack = stack_base; //ebp
	*--stack = 0; //edi
	*--stack = 0; //esi
	if(kernel) {
		*--stack = 0x10; // fs
		*--stack = 0x10; // gs
	} else {
		*--stack = 0x23; // fs
		*--stack = 0x23; // gs
	}

	//Set the ESP to the stack's location
	registers.esp = (size_t) stack;
}

Process::Process(Process *to_fork, Registers &regs){
	if(to_fork->kernel) PANIC("KRNL_PROCESS_FORK", "Kernel processes cannot be forked.",  true);

	_name = to_fork->_name;
	_pid = TaskManager::get_new_pid();
	state = PROCESS_ALIVE;
	kernel = false;
	ring = 3;
	current_brk = to_fork->current_brk;
	cwd = to_fork->cwd;
	parent = to_fork->_pid;

	for(auto i = 0; i < to_fork->file_descriptors.size(); i++)
		file_descriptors.push_back(to_fork->file_descriptors[i]);

	_kernel_stack_base = Paging::PageDirectory::k_alloc_pages(PROCESS_KERNEL_STACK_SIZE);
	_kernel_stack_size = PROCESS_KERNEL_STACK_SIZE;

	page_directory = new Paging::PageDirectory();
	page_directory_loc = page_directory->entries_physaddr();

	//Load the page directory of the new process
	asm volatile("movl %0, %%cr3" :: "r"(page_directory_loc));

	//Fork the old page directory
	page_directory->fork_from(to_fork->page_directory);

	registers.eip = regs.eip;

	auto *stack = (uint32_t*) regs.useresp;

	//Push the correct ss and stack pointer for switching rings during iret
	auto cstack = (size_t) stack;
	*--stack = 0x23;
	*--stack = cstack;

	//Push EFLAGS, CS, and EIP for iret
	*--stack = 0x202; // eflags
	*--stack = kernel ? 0x8 : 0x1B; // cs
	*--stack = regs.eip; // eip

	//We push iret if the pid isn't 1 (the kernel process), because in the preempt_asm function, we return to preempt in
	//the C code. However, when the process is first set up, the ret location isn't in the stack, so we push a function
	//that calls iret.
	*--stack = 0; // fork() in the child returns 0
	*--stack = regs.ebx;
	*--stack = regs.ecx;
	*--stack = regs.edx;
	*--stack = (size_t) TaskManager::proc_first_preempt;

	//Push everything that's popped off in preempt_asm
	*--stack = regs.ebp; //ebp
	*--stack = 0; //edi
	*--stack = 0; //esi
	if(kernel) {
		*--stack = 0x10; // fs
		*--stack = 0x10; // gs
	} else {
		*--stack = 0x23; // fs
		*--stack = 0x23; // gs
	}

	//Set the ESP to the stack's location
	registers.esp = (size_t) stack;

	//Load back the page directory of the current process
	asm volatile("movl %0, %%cr3" :: "r"(TaskManager::current_process()->page_directory_loc));
}

Process::~Process() {
	delete page_directory;
	if(_kernel_stack_base) Paging::PageDirectory::k_free_pages(_kernel_stack_base, PROCESS_KERNEL_STACK_SIZE);
}

void Process::notify(uint32_t sig) {
	switch(sig){
		case SIGTERM:
			kill();
			break;
		case SIGILL:
			printf("\n(PID %d) Illegal operation! Aborting.\n", _pid);
			kill();
			break;
		case SIGSEGV:
			printf("\n(PID %d) Segmentation fault! Aborting.\n", _pid);
			kill();
			break;
	}
}

void Process::kill() {
	if(_pid != 1){
		state = PROCESS_DEAD;
		while(1); //TODO: Figure out a good way to preempt now instead of wasting CPU cycles
	}else{
		PANIC("KERNEL KILLED","EVERYONE PANIC THIS ISN'T GOOD",true);
	}
}

void Process::handle_pagefault(Registers *regs) {
	size_t err_pos;
	asm("mov %%cr2, %0" : "=r" (err_pos));
	if(!page_directory->try_cow(err_pos)) notify(SIGSEGV);
}

void *Process::kernel_stack_top() {
	return (void*)((size_t)_kernel_stack_base + _kernel_stack_size);
}


/************
 * SYSCALLS *
 ************/


ssize_t Process::sys_read(int fd, uint8_t *buf, size_t count) {
	if((size_t)buf + count > HIGHER_HALF) return -EFAULT;
	if(fd < 0 || fd >= file_descriptors.size() || !file_descriptors[fd]) return -EBADF;
	return file_descriptors[fd]->read(buf, count);
}

ssize_t Process::sys_write(int fd, uint8_t *buf, size_t count) {
	if((size_t)buf + count > HIGHER_HALF) return -EFAULT;
	if(fd < 0 || fd >= file_descriptors.size() || !file_descriptors[fd]) return -EBADF;
	return file_descriptors[fd]->write(buf, count);
}

size_t Process::sys_sbrk(int amount) {
	size_t current_brk_page = current_brk / PAGE_SIZE;
	if(amount > 0) {
		size_t new_brk_page = (current_brk + amount) / PAGE_SIZE;
		if(new_brk_page != current_brk_page) {
			page_directory->allocate_pages((current_brk_page + 1) * PAGE_SIZE,
										   (new_brk_page - current_brk_page) * PAGE_SIZE, true);
		}
	} else if (amount < 0) {
		size_t new_brk_page = (current_brk + amount) / PAGE_SIZE;
		if(new_brk_page != current_brk_page) {
			page_directory->deallocate_pages(new_brk_page * PAGE_SIZE, (current_brk_page - new_brk_page) * PAGE_SIZE);
		}
	}
	size_t prev_brk = current_brk;
	current_brk += amount;
	return prev_brk;
}

pid_t Process::sys_fork(Registers& regs) {
	TaskManager::enabled() = false;
	auto* new_proc = new Process(this, regs);
	TaskManager::add_process(new_proc);
	TaskManager::enabled() = true;
	return new_proc->pid();
}

int Process::sys_execve(char *filename, char **argv, char **envp) {
	auto* args = new ProcessArgs(cwd);
	if(argv) {
		int i = 0;
		while(argv[i]) {
			args->argv.push_back(argv[i]);
			i++;
		}
	}
	auto R_new_proc = Process::create_user(filename, args, parent);
	delete args;
	if(R_new_proc.is_error()) return R_new_proc.code();
	R_new_proc.value()->_pid = this->pid();
	TaskManager::add_process(R_new_proc.value());
	kill();
}

int Process::sys_execvp(char *filename, char **argv) {
	auto* args = new ProcessArgs(cwd);
	if(argv) {
		int i = 0;
		while(argv[i]) {
			args->argv.push_back(argv[i]);
			i++;
		}
	}

	ResultRet<Process*> R_new_proc(0);
	if(indexOf('/', filename) == strlen(filename)) {
		R_new_proc = Process::create_user(DC::string("/bin/") + filename, args, parent);
	} else {
		R_new_proc = Process::create_user(filename, args, parent);
	}
	delete args;
	if(R_new_proc.is_error()) return R_new_proc.code();
	R_new_proc.value()->_pid = this->pid();
	TaskManager::add_process(R_new_proc.value());
	kill();
}

int Process::sys_open(char *filename, int options, int mode) {
	auto fd_or_err = VFS::inst().open(filename, options, mode, cwd);
	if(fd_or_err.is_error()) return fd_or_err.code();
	file_descriptors.push_back(fd_or_err.value());
	return (int)file_descriptors.size() - 1;
}

int Process::sys_close(int file) {
	if(file < 0 || file >= file_descriptors.size() || !file_descriptors[file]) return -EBADF;
	file_descriptors[file] = DC::shared_ptr<FileDescriptor>(nullptr);
	return 0;
}

int Process::sys_chdir(char *path) {
	auto inode_or_error = VFS::inst().resolve_path(path, cwd, nullptr);
	if(inode_or_error.is_error()) return inode_or_error.code();
	cwd = inode_or_error.value();
	return 0;
}

int Process::sys_getcwd(char *buf, size_t length) {
	if(cwd->name().length() > length) return -ENAMETOOLONG;
	DC::string path = cwd->get_full_path();
	memcpy(buf, path.c_str(), min(length, path.length()));
	buf[path.length()] = '\0';
	return 0;
}

int Process::sys_readdir(int file, char *buf, size_t len) {
	if(file < 0 || file >= file_descriptors.size() || !file_descriptors[file]) return -EBADF;
	return file_descriptors[file]->read_dir_entries(buf, len);
}

int Process::sys_fstat(int file, char *buf) {
	if(file < 0 || file >= file_descriptors.size() || !file_descriptors[file]) return -EBADF;
	file_descriptors[file]->metadata().stat((struct stat*)buf);
	return 0;
}

int Process::sys_stat(char *file, char *buf) {
	auto inode_or_err = VFS::inst().resolve_path(file, cwd);
	if(inode_or_err.is_error()) return inode_or_err.code();
	inode_or_err.value()->inode()->metadata().stat((struct stat*)buf);
	return 0;
}

int Process::sys_lseek(int file, off_t off, int whence) {
	if(file < 0 || file >= file_descriptors.size() || !file_descriptors[file]) return -EBADF;
	return file_descriptors[file]->seek(off, whence);
}

int Process::sys_waitpid(pid_t pid, int* status, int flags) {
	if(pid < -1) {
		return -ECHILD; //TODO: Wait for process with pgroup abs(pid)
	} else if(pid == -1) {
		return -ECHILD; //TODO: Wait for any child
	} else if(pid == 0) {
		return -ECHILD; //TODO: Wait for process in same pgroup
	} else {
		if(!TaskManager::process_for_pid(pid)) return -ECHILD;
		while(TaskManager::process_for_pid(pid)); //TODO: Better way of hanging without wasting CPU cycles
		if(status)
			*status = 0; //TODO: Process status
	}

	return pid;
}
