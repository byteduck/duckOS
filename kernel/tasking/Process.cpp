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
#include <kernel/pit.h>
#include "Process.h"
#include "TaskManager.h"
#include "elf.h"
#include "ProcessArgs.h"
#include <kernel/memory/PageDirectory.h>
#include <kernel/interrupt/syscall.h>

Process* Process::create_kernel(const DC::string& name, void (*func)()){
	ProcessArgs args = ProcessArgs(DC::shared_ptr<LinkedInode>(nullptr));
	return new Process(name, (size_t)func, true, &args, 0);
}

ResultRet<Process*> Process::create_user(const DC::string& executable_loc, ProcessArgs* args, pid_t parent) {
	auto fd_or_error = VFS::inst().open((DC::string&) executable_loc, O_RDONLY, 0, args->working_dir);
	if(fd_or_error.is_error()) return fd_or_error.code();

	auto fd = fd_or_error.value();
	auto* header = new ELF::elf32_header;
	fd->read((uint8_t*)header, sizeof(ELF::elf32_header));
	if(!ELF::can_execute(header)) {
		delete header;
		return -ENOEXEC;
	}

	auto* proc = new Process(executable_loc, header->program_entry_position, false, args, parent);
	bool success = proc->load_elf(fd, header);

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

	for(uint32_t i = 0; i < num_pheaders; i++) {
		auto pheader = &program_headers[i];
		if(pheader->p_type == ELF_PT_LOAD) {
			size_t loadloc_pagealigned = (pheader->p_vaddr/PAGE_SIZE) * PAGE_SIZE;
			size_t loadsize_pagealigned = pheader->p_memsz + (pheader->p_vaddr % PAGE_SIZE);

			//Allocate a kernel memory region to load the section into
			LinkedMemoryRegion tmp_region = PageDirectory::k_alloc_region(loadsize_pagealigned);

			//Read the section into the region
			fd->seek(pheader->p_offset, SEEK_SET);
			fd->read((uint8_t*) tmp_region.virt->start + (pheader->p_vaddr - loadloc_pagealigned), pheader->p_filesz);

			//Allocate a program vmem region
			MemoryRegion* vmem_region = page_directory->vmem_map().allocate_region(loadloc_pagealigned, loadsize_pagealigned);
			if(!vmem_region) {
				//If we failed to allocate the program vmem region, free the tmp region
				Memory::pmem_map().free_region(tmp_region.phys);
				printf("FATAL: Failed to allocate a vmem region in load_elf!\n");
				break;
			}

			//Unmap the region from the kernel
			PageDirectory::k_unmap_region(tmp_region);
			PageDirectory::kernel_vmem_map.free_region(tmp_region.virt);

			//Map the physical region to the program's vmem region
			LinkedMemoryRegion prog_region(tmp_region.phys, vmem_region);
			page_directory->map_region(prog_region, pheader->p_flags & ELF_PF_W);

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

Process::Process(const DC::string& name, size_t entry_point, bool kernel, ProcessArgs* args, pid_t parent) {
	//Disable task switching so we don't screw up paging
	bool en = TaskManager::enabled();
	TaskManager::enabled() = false;

	_name = name;
	_pid = TaskManager::get_new_pid();
	state = PROCESS_ALIVE;
	this->kernel = kernel;
	ring = kernel ? 0 : 3;
	parent = parent;
	quantum = 1;

	if(!kernel) {
		auto ttydesc = DC::make_shared<FileDescriptor>(TTYDevice::current_tty());
		file_descriptors = DC::vector<DC::shared_ptr<FileDescriptor>>(3);
		file_descriptors.push_back(ttydesc);
		file_descriptors.push_back(ttydesc);
		file_descriptors.push_back(ttydesc);
		cwd = args->working_dir;
	}

	_kernel_stack_base = (void*) PageDirectory::k_alloc_region(PROCESS_KERNEL_STACK_SIZE).virt->start;
	_kernel_stack_size = PROCESS_KERNEL_STACK_SIZE;

	size_t stack_base;
	if(!kernel) {
		//Make and load a new page directory
		page_directory = new PageDirectory();
		page_directory_loc = page_directory->entries_physaddr();
		asm volatile("movl %0, %%cr3" :: "r"(page_directory_loc));

		if(!page_directory->allocate_region(HIGHER_HALF - PROCESS_STACK_SIZE, PROCESS_STACK_SIZE, true).virt)
			PANIC("NEW_PROC_STACK_ALLOC_FAIL", "Was unable to allocate virtual memory for a new process's stack.", true);

		stack_base = HIGHER_HALF - PROCESS_STACK_SIZE;
		_stack_size = PROCESS_STACK_SIZE;
	} else {
		page_directory_loc = Memory::kernel_page_directory.entries_physaddr();
		stack_base = (size_t) _kernel_stack_base;
		_stack_size = PROCESS_KERNEL_STACK_SIZE;
	}

	//Setup registers
	registers.eflags = 0x202;
	registers.cs = kernel ? 0x8 : 0x1B;
	registers.eip = entry_point;
	registers.eax = 0;
	registers.ebx = 0;
	registers.ecx = 0;
	registers.edx = 0;
	registers.ebp = stack_base;
	registers.edi = 0;
	registers.esi = 0;
	if(kernel) {
		registers.ds = 0x10; // ds
		registers.es = 0x10; // es
		registers.fs = 0x10; // fs
		registers.gs = 0x10; // gs
	} else {
		registers.ds = 0x23; // ds
		registers.es = 0x23; // es
		registers.fs = 0x23; // fs
		registers.gs = 0x23; // gs
	}

	//Setup stack
	if(ring != 0) {
		//Set up the user stack for the program arguments
		auto *user_stack = (uint32_t *) (stack_base + _stack_size);
		user_stack = (uint32_t *) args->setup_stack(user_stack);
		*--user_stack = 0; //Honestly? Not sure why this is needed but nothing works without it :)

		//Setup the kernel stack with register states
		auto *kernel_stack = (uint32_t *) ((size_t) _kernel_stack_base + _kernel_stack_size);
		setup_stack(kernel_stack, user_stack, registers);
	} else {
		auto *stack = (uint32_t*) (stack_base + _stack_size);
		stack = (uint32_t *) args->setup_stack(stack);
		*--stack = 0;
		setup_stack(stack, stack, registers);
	}

	if(!kernel) {
		//Load back the page directory of the current process
		asm volatile("movl %0, %%cr3" :: "r"(TaskManager::current_process()->page_directory_loc));
	}

	TaskManager::enabled() = en;
}

Process::Process(Process *to_fork, Registers &regs){
	if(to_fork->kernel) PANIC("KRNL_PROCESS_FORK", "Kernel processes cannot be forked.",  true);

	TaskManager::enabled() = false;
	_name = to_fork->_name;
	_pid = TaskManager::get_new_pid();
	state = PROCESS_ALIVE;
	kernel = false;
	ring = 3;
	current_brk = to_fork->current_brk;
	cwd = to_fork->cwd;
	parent = to_fork->_pid;
	quantum = to_fork->quantum;

	//Copy signal handlers and file descriptors
	for(size_t i = 0; i < to_fork->file_descriptors.size(); i++)
		file_descriptors.push_back(to_fork->file_descriptors[i]);
	for(int i = 0; i < 32; i++)
		signal_actions[i] = to_fork->signal_actions[i];

	_kernel_stack_base = (void*) PageDirectory::k_alloc_region(PROCESS_KERNEL_STACK_SIZE).virt->start;
	_kernel_stack_size = PROCESS_KERNEL_STACK_SIZE;

	page_directory = new PageDirectory();
	page_directory_loc = page_directory->entries_physaddr();

	//Load the page directory of the new process
	asm volatile("movl %0, %%cr3" :: "r"(page_directory_loc));

	//Fork the old page directory
	page_directory->fork_from(to_fork->page_directory);

	//Setup registers and stack
	registers = regs;
	registers.eax = 0; // fork() in child returns zero
	auto* user_stack = (size_t*) regs.useresp;
	auto* kernel_stack = (size_t*) ((size_t) _kernel_stack_base + _kernel_stack_size);
	setup_stack(kernel_stack, user_stack, registers);

	//Load back the page directory of the current process
	asm volatile("movl %0, %%cr3" :: "r"(TaskManager::current_process()->page_directory_loc));

	TaskManager::enabled() = true;
}

void Process::setup_stack(uint32_t*& kernel_stack, uint32_t* userstack, Registers& regs) {
	//If usermode, push ss and useresp
	if(ring != 0) {
		*--kernel_stack = 0x23;
		*--kernel_stack = (size_t) userstack;
	}

	//Push EFLAGS, CS, and EIP for iret
	*--kernel_stack = regs.eflags; // eflags
	*--kernel_stack = regs.cs; // cs
	*--kernel_stack = regs.eip; // eip

	*--kernel_stack = regs.eax;
	*--kernel_stack = regs.ebx;
	*--kernel_stack = regs.ecx;
	*--kernel_stack = regs.edx;
	*--kernel_stack = regs.ebp;
	*--kernel_stack = regs.edi;
	*--kernel_stack = regs.esi;
	*--kernel_stack = regs.ds;
	*--kernel_stack = regs.es;
	*--kernel_stack = regs.fs;
	*--kernel_stack = regs.gs;

	if(_pid != 1) {
		*--kernel_stack = (size_t) TaskManager::proc_first_preempt;
		*--kernel_stack = 0; //Fake popped EBP
	}

	regs.esp = (size_t) kernel_stack;
	regs.useresp = (size_t) userstack;
}

Process::~Process() {
	delete page_directory;
	if(_kernel_stack_base) PageDirectory::k_free_region(_kernel_stack_base);
}

void Process::kill(int signal, bool notify_yielders) {
	pending_signals.push(signal);
	if(!notify_yielders) notify_yielders_on_death = false;
	if(TaskManager::current_process() == this) TaskManager::yield();
}

bool Process::handle_pending_signal() {
	if(pending_signals.empty() || _in_signal) return false;
	int signal = pending_signals.pop();
	if(signal >= 0 && signal <= 32) {
		Signal::SignalSeverity severity = Signal::signal_severities[signal];

		//Print signal if unhandled and fatal
		if(severity == Signal::FATAL && !signal_actions[signal].action)
			printf("PID %d exiting with signal %s\n", _pid, Signal::signal_names[signal]);

		if(severity >= Signal::KILL && !signal_actions[signal].action) {
			//If the signal has no handler and is KILL or FATAL, then kill the process
			if (notify_yielders_on_death) _yielder.set_all_ready();
			state = PROCESS_DEAD;
			pending_signals.pop();
		} else if(signal_actions[signal].action) {
			//Otherwise, have the process handle the signal
			call_signal_handler(signal);
		}
	}
	return true;
}

bool Process::has_pending_signals() {
	return !pending_signals.empty();
}

void Process::call_signal_handler(int signal) {
	if(signal < 1 || signal >= 32) return;
	auto signal_loc = (size_t) signal_actions[signal].action;
	if(!signal_loc || signal_loc >= HIGHER_HALF) return;

	//Load the page directory so we can write to the userspace stack
	asm volatile("mov %0, %%cr3" :: "r"(page_directory_loc));

	//Allocate a userspace stack
	_sighandler_ustack_region = page_directory->allocate_region(HIGHER_HALF - PROCESS_STACK_SIZE * 2, PROCESS_STACK_SIZE, true);
	if(!_sighandler_ustack_region.virt) {
		printf("FATAL: Failed to allocate sighandler user stack for pid %d!\n", pid());
		kill(SIGKILL);
		return;
	}

	//Allocate a kernel stack
	_sighandler_kstack_region = PageDirectory::k_alloc_region(PROCESS_KERNEL_STACK_SIZE);
	if(!_sighandler_kstack_region.virt) {
		printf("FATAL: Failed to allocate sighandler kernel stack for pid %d!\n", pid());
		kill(SIGKILL);
		return;
	}

	auto* user_stack = (size_t*) (_sighandler_ustack_region.virt->start + PROCESS_STACK_SIZE);
	_signal_stack_top = _sighandler_kstack_region.virt->start + _sighandler_kstack_region.virt->size;

	//Push signal number and fake return address to the stack
	*--user_stack = signal;
	*--user_stack = SIGNAL_RETURN_FAKE_ADDR;

	//Setup signal registers
	signal_registers.eflags = 0x202;
	signal_registers.cs = ring == 0 ? 0x8 : 0x1B;
	signal_registers.eip = signal_loc;
	signal_registers.eax = 0;
	signal_registers.ebx = 0;
	signal_registers.ecx = 0;
	signal_registers.edx = 0;
	signal_registers.ebp = (size_t) user_stack;
	signal_registers.edi = 0;
	signal_registers.esi = 0;
	if(ring == 0) {
		signal_registers.ds = 0x10; // ds
		signal_registers.es = 0x10; // es
		signal_registers.fs = 0x10; // fs
		signal_registers.gs = 0x10; // gs
	} else {
		signal_registers.ds = 0x23; // ds
		signal_registers.es = 0x23; // es
		signal_registers.fs = 0x23; // fs
		signal_registers.gs = 0x23; // gs
	}

	//Setup signal stack
	setup_stack(user_stack, user_stack, signal_registers);

	//Load back page directory of current process
	asm volatile("mov %0, %%cr3" :: "r"(TaskManager::current_process()->page_directory_loc));

	_ready_to_handle_signal = true;
}

bool& Process::in_signal_handler() {
	return _in_signal;
}

bool& Process::just_finished_signal() {
	return _just_finished_signal;
}

bool& Process::ready_to_handle_signal() {
	return _ready_to_handle_signal;
}

void* Process::signal_stack_top() {
	return (void*) _signal_stack_top;
}

void Process::handle_pagefault(Registers *regs) {
	size_t err_pos;
	asm volatile ("mov %%cr2, %0" : "=r" (err_pos));
	//If the fault is at the fake signal return address, exit the signal handler
	if(_in_signal && err_pos == SIGNAL_RETURN_FAKE_ADDR) {
		_just_finished_signal = true;
		PageDirectory::k_free_region(_sighandler_kstack_region);
		page_directory->free_region(_sighandler_ustack_region);
		TaskManager::yield();
	}
	if(!page_directory->try_cow(err_pos)) kill(SIGSEGV);
}

void *Process::kernel_stack_top() {
	return (void*)((size_t)_kernel_stack_base + _kernel_stack_size);
}

void Process::yield_to(TaskYieldQueue& yielder) {
	ASSERT(TaskManager::enabled());
	yielder.add_process(this);
	_yielding_to = &yielder;
	state = PROCESS_YIELDING;
	TaskManager::yield();
	_yielding_to = nullptr;
	state = PROCESS_ALIVE;
}

void Process::yield_to(Process* proc) {
	yield_to(proc->_yielder);
}

bool Process::is_yielding() {
	return _yielding_to != nullptr;
}

TaskYieldQueue* Process::yielding_to() {
	return _yielding_to;
}

void Process::finish_yielding() {
	_yielding_to = nullptr;
}


/************
 * SYSCALLS *
 ************/

void Process::check_ptr(void *ptr) {
	if(ptr != nullptr && (size_t) ptr >= HIGHER_HALF) {
		kill(SIGSEGV);
	}
}

ssize_t Process::sys_read(int fd, uint8_t *buf, size_t count) {
	check_ptr(buf);
	if(fd < 0 || fd >= (int) file_descriptors.size() || !file_descriptors[fd]) return -EBADF;
	int ret =  file_descriptors[fd]->read(buf, count);
	return ret;
}

ssize_t Process::sys_write(int fd, uint8_t *buf, size_t count) {
	check_ptr(buf);
	if(fd < 0 || fd >= (int) file_descriptors.size() || !file_descriptors[fd]) return -EBADF;
	return file_descriptors[fd]->write(buf, count);
}

size_t Process::sys_sbrk(int amount) {
	size_t current_brk_page = current_brk / PAGE_SIZE;
	if(amount > 0) {
		size_t new_brk_page = (current_brk + amount) / PAGE_SIZE;
		if(new_brk_page != current_brk_page) {
			page_directory->allocate_region((current_brk_page + 1) * PAGE_SIZE,
										   (new_brk_page - current_brk_page) * PAGE_SIZE, true);
		}
	} else if (amount < 0) {
		size_t new_brk_page = (current_brk + amount) / PAGE_SIZE;
		if(new_brk_page != current_brk_page) {
			page_directory->free_region((void*) (new_brk_page * PAGE_SIZE));
		}
	}
	size_t prev_brk = current_brk;
	current_brk += amount;
	return prev_brk;
}

pid_t Process::sys_fork(Registers& regs) {
	auto* new_proc = new Process(this, regs);
	TaskManager::add_process(new_proc);
	return new_proc->pid();
}

int Process::exec(const DC::string& filename, ProcessArgs* args) {
	auto R_new_proc = Process::create_user(filename, args, parent);
	delete args;
	filename.~string(); //Manually delete because we won't return from here and we need to clean up resources
	if(R_new_proc.is_error()) return R_new_proc.code();
	R_new_proc.value()->_pid = this->pid();
	R_new_proc.value()->_yielder = this->_yielder;
	TaskManager::add_process(R_new_proc.value());
	kill(SIGKILL, false);
	ASSERT(false)
	return -1;
}

int Process::sys_execve(char *filename, char **argv, char **envp) {
	check_ptr(filename);
	check_ptr(argv);
	check_ptr(envp);
	auto* args = new ProcessArgs(cwd);
	if(argv) {
		int i = 0;
		while(argv[i]) {
			check_ptr(argv[i]);
			args->argv.push_back(argv[i]);
			i++;
		}
	}
	return exec(filename, args);
}

int Process::sys_execvp(char *filename, char **argv) {
	check_ptr(filename);
	check_ptr(argv);
	auto* args = new ProcessArgs(cwd);
	if(argv) {
		int i = 0;
		while(argv[i]) {
			check_ptr(argv[i]);
			args->argv.push_back(argv[i]);
			i++;
		}
	}
	if(indexOf('/', filename) == strlen(filename)) {
		return exec(DC::string("/bin/") + filename, args);
	} else {
		return exec(filename, args);
	}
}

int Process::sys_open(char *filename, int options, int mode) {
	check_ptr(filename);
	DC::string path = filename;
	mode &= 04777; //We just want the permission bits
	auto fd_or_err = VFS::inst().open(path, options, mode, cwd);
	if(fd_or_err.is_error()) return fd_or_err.code();
	file_descriptors.push_back(fd_or_err.value());
	return (int)file_descriptors.size() - 1;
}

int Process::sys_close(int file) {
	if(file < 0 || file >= (int) file_descriptors.size() || !file_descriptors[file]) return -EBADF;
	file_descriptors[file] = DC::shared_ptr<FileDescriptor>(nullptr);
	return 0;
}

int Process::sys_chdir(char *path) {
	check_ptr(path);
	DC::string strpath = path;
	auto inode_or_error = VFS::inst().resolve_path(strpath, cwd, nullptr);
	if(inode_or_error.is_error()) return inode_or_error.code();
	cwd = inode_or_error.value();
	return 0;
}

int Process::sys_getcwd(char *buf, size_t length) {
	check_ptr(buf);
	if(cwd->name().length() > length) return -ENAMETOOLONG;
	DC::string path = cwd->get_full_path();
	memcpy(buf, path.c_str(), min(length, path.length()));
	buf[path.length()] = '\0';
	return 0;
}

int Process::sys_readdir(int file, char *buf, size_t len) {
	check_ptr(buf);
	TaskManager::yield();
	if(file < 0 || file >= (int) file_descriptors.size() || !file_descriptors[file]) return -EBADF;
	return file_descriptors[file]->read_dir_entries(buf, len);
}

int Process::sys_fstat(int file, char *buf) {
	if(file < 0 || file >= (int) file_descriptors.size() || !file_descriptors[file]) return -EBADF;
	file_descriptors[file]->metadata().stat((struct stat*)buf);
	return 0;
}

int Process::sys_stat(char *file, char *buf) {
	check_ptr(file);
	check_ptr(buf);
	DC::string path(file);
	auto inode_or_err = VFS::inst().resolve_path(path, cwd);
	if(inode_or_err.is_error()) return inode_or_err.code();
	inode_or_err.value()->inode()->metadata().stat((struct stat*)buf);
	return 0;
}

int Process::sys_lseek(int file, off_t off, int whence) {
	if(file < 0 || file >= (int) file_descriptors.size() || !file_descriptors[file]) return -EBADF;
	return file_descriptors[file]->seek(off, whence);
}

int Process::sys_waitpid(pid_t pid, int* status, int flags) {
	check_ptr(status);
	if(pid < -1) {
		return -ECHILD; //TODO: Wait for process with pgroup abs(pid)
	} else if(pid == -1) {
		return -ECHILD; //TODO: Wait for any child
	} else if(pid == 0) {
		return -ECHILD; //TODO: Wait for process in same pgroup
	} else {
		Process* proc = TaskManager::process_for_pid(pid);
		if(!proc) return -ECHILD;
		yield_to(proc);
		if(status)
			*status = 0; //TODO: Process status
	}

	return pid;
}

int Process::sys_gettimeofday(timespec *t, void *z) {
	check_ptr(t);
	check_ptr(z);
	PIT::gettimeofday(t, z);
	return 0;
}

int Process::sys_sigaction(int sig, sigaction_t *new_action, sigaction_t *old_action) {
	check_ptr(new_action);
	check_ptr(old_action);
	check_ptr((void*) new_action->sa_sigaction);
	if(sig == SIGSTOP || sig == SIGKILL || sig < 1 || sig >= 32) return -EINVAL;
	cli(); //We don't want this interrupted or else we'd have a problem if it's needed before it's done
	if(old_action) {
		memcpy(&old_action->sa_sigaction, &signal_actions[sig].action, sizeof(Signal::SigAction::action));
		memcpy(&old_action->sa_flags, &signal_actions[sig].flags, sizeof(Signal::SigAction::flags));
	}
	signal_actions[sig].action = new_action->sa_sigaction;
	signal_actions[sig].flags = new_action->sa_flags;
	sti();
	return 0;
}

int Process::sys_kill(pid_t pid, int sig) {
	//TODO: Permission check
	if(sig == 0) return 0;
	if(sig < 0 || sig >= NSIG) return -EINVAL;
	if(pid == _pid) kill(sig);
	else if(pid == 0) {
		//TODO: kill processes in pgroup
	} else if(pid == -1) {
		//TODO: kill all processes for which we have permission to kill
	} else if(pid < -1) {
		//TODO: kill all processes in pgroup -pid
	} else {
		Process* proc = TaskManager::process_for_pid(pid);
		if(!proc) return -ESRCH;
		proc->kill(sig);
	}
	return 0;
}

int Process::sys_unlink(char* name) {
	check_ptr(name);
	DC::string path(name);
	auto ret = VFS::inst().unlink(path, cwd);
	if(ret.is_error()) return ret.code();
	return 0;
}

int Process::sys_rmdir(char* name) {
	check_ptr(name);
	DC::string path(name);
	auto ret = VFS::inst().rmdir(path, cwd);
	if(ret.is_error()) return ret.code();
	return 0;
}

int Process::sys_mkdir(char *path, mode_t mode) {
	check_ptr(path);
	DC::string strpath(path);
	auto ret = VFS::inst().mkdir(strpath, mode, cwd);
	if(ret.is_error()) return ret.code();
	return 0;
}

int Process::sys_mkdirat(int fd, char *path, mode_t mode) {
	return -1;
}
