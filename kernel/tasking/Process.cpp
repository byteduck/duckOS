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

#include <kernel/filesystem/VFS.h>
#include <common/defines.h>
#include <kernel/terminal/TTYDevice.h>
#include <kernel/pit.h>
#include "Process.h"
#include "TaskManager.h"
#include "ELF.h"
#include "ProcessArgs.h"
#include "WaitBlocker.h"
#include "PollBlocker.h"
#include <kernel/memory/PageDirectory.h>
#include <kernel/interrupt/syscall.h>
#include <kernel/terminal/VirtualTTY.h>
#include <kernel/terminal/PTYControllerDevice.h>
#include <kernel/terminal/PTYDevice.h>

const char* PROC_STATUS_NAMES[] = {"Alive", "Zombie", "Dead", "Sleeping"};

Process* Process::create_kernel(const DC::string& name, void (*func)()){
	ProcessArgs args = ProcessArgs(DC::shared_ptr<LinkedInode>(nullptr));
	return new Process(name, (size_t)func, true, &args, 1);
}

ResultRet<Process*> Process::create_user(const DC::string& executable_loc, User& file_open_user, ProcessArgs* args, pid_t parent) {
	//Open the executable
	auto fd_or_error = VFS::inst().open((DC::string&) executable_loc, O_RDONLY, 0, file_open_user, args->working_dir);
	if(fd_or_error.is_error())
		return fd_or_error.code();
	auto fd = fd_or_error.value();

	//Read info
	auto info_or_err = ELF::read_info(fd, file_open_user);
	if(info_or_err.is_error())
		return info_or_err.code();
	auto info = info_or_err.value();

	//If there's an interpreter, we need to change the arguments accordingly
	if(info.interpreter.length()) {
		//Get the full path of the program we're trying to run
		auto resolv = VFS::inst().resolve_path((DC::string&) executable_loc, args->working_dir, file_open_user);
		if(resolv.is_error())
			return resolv.code();

		//Add the interpreter + the full path of the program to the args
		auto new_argv = DC::vector<DC::string>();
		new_argv.push_back(info.interpreter);
		new_argv.push_back(resolv.value()->get_full_path());

		//Add the old arguments to the new argv and replace the argv
		for(size_t i = 0; i < args->argv.size(); i++)
			new_argv.push_back(args->argv[i]);
		args->argv = new_argv;
	}

	//Create the process
	auto* proc = new Process(VFS::path_base(executable_loc), info.header->program_entry_position, false, args, parent);
	proc->_exe = executable_loc;

	//Load the ELF into the process's page directory and set proc->current_brk
	auto brk_or_err = ELF::load_sections(*info.fd, info.segments, proc->page_directory);
	if(brk_or_err.is_error())
		return brk_or_err.code();

	return proc;
}

pid_t Process::pid() {
	return _pid;
}

pid_t Process::pgid() {
	return _pgid;
}

pid_t Process::ppid() {
	return _ppid;
}

void Process::set_ppid(pid_t ppid) {
	_ppid = ppid;
}

pid_t Process::sid() {
	return _sid;
}

User Process::user() {
	return _user;
}

DC::string Process::name(){
	return _name;
}

DC::string Process::exe() {
	return _exe;
}

DC::shared_ptr<LinkedInode> Process::cwd() {
	return _cwd;
}

void Process::set_tty(TTYDevice* tty) {
	_tty = tty;
}

int Process::exit_status() {
	return _exit_status;
}

Process::Process(const DC::string& name, size_t entry_point, bool kernel, ProcessArgs* args, pid_t ppid): _user(User::root()) {
	//Disable task switching so we don't screw up paging
	bool en = TaskManager::enabled();
	TaskManager::enabled() = false;

	_name = name;
	_pid = TaskManager::get_new_pid();
	state = PROCESS_ALIVE;
	this->kernel = kernel;
	ring = kernel ? 0 : 3;
	_ppid = _pid > 1 ? ppid : 0; //kidle and init have a parent of 0
	quantum = 1;

	if(!kernel) {
		auto ttydesc = DC::make_shared<FileDescriptor>(VirtualTTY::current_tty());
		ttydesc->set_owner(this);
		ttydesc->set_options(O_RDWR);
		_file_descriptors.push_back(ttydesc);
		_file_descriptors.push_back(ttydesc);
		_file_descriptors.push_back(ttydesc);
		_cwd = args->working_dir;
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
		auto user_stack = (uint32_t *) (stack_base + _stack_size);
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

Process::Process(Process *to_fork, Registers &regs): _user(to_fork->_user) {
	if(to_fork->kernel)
		PANIC("KRNL_PROCESS_FORK", "Kernel processes cannot be forked.",  true);

	TaskManager::enabled() = false;
	_name = to_fork->_name;
	_pid = TaskManager::get_new_pid();
	state = PROCESS_ALIVE;
	kernel = false;
	ring = 3;
	_cwd = to_fork->_cwd;
	_ppid = to_fork->_pid;
	_sid = to_fork->_sid;
	_pgid = to_fork->_pgid;
	_umask = to_fork->_umask;
	quantum = to_fork->quantum;

	//Copy signal handlers and file descriptors
	for(int i = 0; i < 32; i++)
		signal_actions[i] = to_fork->signal_actions[i];

	_file_descriptors.resize(to_fork->_file_descriptors.size());
	for(size_t i = 0; i < to_fork->_file_descriptors.size(); i++) {
		if(to_fork->_file_descriptors[i]) {
			_file_descriptors[i] = DC::make_shared<FileDescriptor>(*to_fork->_file_descriptors[i]);
			_file_descriptors[i]->set_owner(this);
		}
	}

	//Allocate kernel stack
	_kernel_stack_base = (void*) PageDirectory::k_alloc_region(PROCESS_KERNEL_STACK_SIZE).virt->start;
	_kernel_stack_size = PROCESS_KERNEL_STACK_SIZE;

	//Create page directory
	page_directory = new PageDirectory();
	page_directory_loc = page_directory->entries_physaddr();

	//Load the page directory of the new process
	asm volatile("movl %0, %%cr3" :: "r"(page_directory_loc));

	//Fork the old page directory
	page_directory->fork_from(to_fork->page_directory, _ppid, _pid);

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

	if(_pid != 0) {
		*--kernel_stack = (size_t) TaskManager::proc_first_preempt;
		*--kernel_stack = 0; //Fake popped EBP
	}

	regs.esp = (size_t) kernel_stack;
	regs.useresp = (size_t) userstack;
}

Process::~Process() {
	free_resources();
}

void Process::free_resources() {
	if(_freed_resources)
		return;
	LOCK(_lock);
	_freed_resources = true;
	delete page_directory;
	if(_kernel_stack_base)
		PageDirectory::k_free_region(_kernel_stack_base);
	_file_descriptors.resize(0);
}

void Process::reap() {
	LOCK(_lock);
	if(state != PROCESS_ZOMBIE) return;
	state = PROCESS_DEAD;
}

void Process::kill(int signal) {
	pending_signals.push(signal);
	if(TaskManager::current_process() == this)
		TaskManager::yield();
}

void Process::die_silently() {
	state = PROCESS_DEAD;
	if(TaskManager::current_process() == this)
		TaskManager::yield();
}

bool Process::handle_pending_signal() {
	if(pending_signals.empty() || _in_signal)
		return false;
	int signal = pending_signals.front();
	pending_signals.pop();
	if(signal >= 0 && signal <= 32) {
		Signal::SignalSeverity severity = Signal::signal_severities[signal];

		//Print signal if unhandled and fatal
		if(severity == Signal::FATAL && !signal_actions[signal].action)
			printf("PID %d exiting with signal %s\n", _pid, Signal::signal_names[signal]);

		if(severity >= Signal::KILL && !signal_actions[signal].action) {
			//If the signal has no handler and is KILL or FATAL, then kill the process
			free_resources();
			Process* parent = TaskManager::process_for_pid(_ppid);
			if(parent && parent != this)
				parent->kill(SIGCHLD);
			state = PROCESS_ZOMBIE;
			TaskManager::reparent_orphans(this);
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
	//TODO: Do this without switching page directories
	if(signal < 1 || signal >= 32)
		return;
	auto signal_loc = (size_t) signal_actions[signal].action;
	if(!signal_loc || signal_loc >= HIGHER_HALF)
		return;

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
	if(!page_directory->try_cow(err_pos)) {
		printf("SIGSEGV AT %x %x\n", err_pos, regs->eip);
		kill(SIGSEGV);
	}
}

void *Process::kernel_stack_top() {
	return (void*)((size_t)_kernel_stack_base + _kernel_stack_size);
}

void Process::block(Blocker& blocker) {
	ASSERT(state == PROCESS_ALIVE);
	ASSERT(!_blocker);
	state = PROCESS_BLOCKED;
	_blocker = &blocker;
	_blocker->assign_process(this);
	TaskManager::yield();
}

void Process::unblock() {
	_blocker->clear_process();
	_blocker = nullptr;
	state = PROCESS_ALIVE;
}

bool Process::is_blocked() {
	return state == PROCESS_BLOCKED;
}

bool Process::should_unblock() {
	return _blocker && _blocker->is_ready();
}


/************
 * SYSCALLS *
 ************/

void Process::check_ptr(const void *ptr) {
	if((size_t) ptr >= HIGHER_HALF || !page_directory->is_mapped((size_t) ptr)) {
		kill(SIGSEGV);
	}
}

void Process::sys_exit(int status) {
	_exit_status = status;
}

ssize_t Process::sys_read(int fd, uint8_t *buf, size_t count) {
	check_ptr(buf);
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd])
		return -EBADF;
	return _file_descriptors[fd]->read(buf, count);
}

ssize_t Process::sys_write(int fd, uint8_t *buf, size_t count) {
	check_ptr(buf);
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd])
		return -EBADF;
	ssize_t ret = _file_descriptors[fd]->write(buf, count);
	return ret;
}

pid_t Process::sys_fork(Registers& regs) {
	auto* new_proc = new Process(this, regs);
	TaskManager::add_process(new_proc);
	return new_proc->pid();
}

int Process::exec(const DC::string& filename, ProcessArgs* args) {
	//Create the new process
	auto R_new_proc = Process::create_user(filename, _user, args, _ppid);
	if(R_new_proc.is_error())
		return R_new_proc.code();
	auto* new_proc = R_new_proc.value();

	//Properly set new process's PID, blocker, and stdout/in/err
	new_proc->_pid = _pid;
	new_proc->_user = _user;
	new_proc->_pgid = _pgid;
	new_proc->_sid = _sid;
	new_proc->_blocker = this->_blocker;
	if(kernel) {
		//Kernel processes have no file descriptors, so we need to initialize them
		auto ttydesc = DC::make_shared<FileDescriptor>(VirtualTTY::current_tty());
		ttydesc->set_owner(this);
		ttydesc->set_options(O_RDWR);
		_file_descriptors.resize(0); //Just in case
		_file_descriptors.push_back(ttydesc);
		_file_descriptors.push_back(ttydesc);
		_file_descriptors.push_back(ttydesc);
		_cwd = args->working_dir;
	}

	//Give new process first three file descriptors and close the rest
	for(size_t i = 0; i < 3; i++) {
		new_proc->_file_descriptors[i] = _file_descriptors[i];
		new_proc->_file_descriptors[i]->set_owner(new_proc);
	}
	_file_descriptors.resize(0);

	//Manually delete because we won't return from here and we need to clean up resources
	delete args;
	filename.~string();

	//Add the new process to the process list
	cli();
	_pid = -1;
	state = PROCESS_DEAD;
	TaskManager::add_process(new_proc);
	TaskManager::yield();
	ASSERT(false);
	return -1;
}

int Process::sys_execve(char *filename, char **argv, char **envp) {
	check_ptr(filename);
	check_ptr(argv);
	check_ptr(envp);
	auto* args = new ProcessArgs(_cwd);
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
	auto* args = new ProcessArgs(_cwd);
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
	auto fd_or_err = VFS::inst().open(path, options, mode & (~_umask), _user, _cwd);
	if(fd_or_err.is_error())
		return fd_or_err.code();
	_file_descriptors.push_back(fd_or_err.value());
	fd_or_err.value()->set_owner(this);
	return (int)_file_descriptors.size() - 1;
}

int Process::sys_close(int file) {
	if(file < 0 || file >= (int) _file_descriptors.size() || !_file_descriptors[file])
		return -EBADF;
	_file_descriptors[file] = DC::shared_ptr<FileDescriptor>(nullptr);
	return 0;
}

int Process::sys_chdir(char *path) {
	check_ptr(path);
	DC::string strpath = path;
	auto inode_or_error = VFS::inst().resolve_path(strpath, _cwd, _user);
	if(inode_or_error.is_error())
		return inode_or_error.code();
	if(!inode_or_error.value()->inode()->metadata().is_directory())
		return -ENOTDIR;
	_cwd = inode_or_error.value();
	return SUCCESS;
}

int Process::sys_getcwd(char *buf, size_t length) {
	check_ptr(buf);
	if(_cwd->name().length() > length)
		return -ENAMETOOLONG;
	DC::string path = _cwd->get_full_path();
	memcpy(buf, path.c_str(), min(length, path.length()));
	buf[path.length()] = '\0';
	return 0;
}

int Process::sys_readdir(int file, char *buf, size_t len) {
	check_ptr(buf);
	if(file < 0 || file >= (int) _file_descriptors.size() || !_file_descriptors[file])
		return -EBADF;
	return _file_descriptors[file]->read_dir_entries(buf, len);
}

int Process::sys_fstat(int file, char *buf) {
	if(file < 0 || file >= (int) _file_descriptors.size() || !_file_descriptors[file])
		return -EBADF;
	_file_descriptors[file]->metadata().stat((struct stat*)buf);
	return 0;
}

int Process::sys_stat(char *file, char *buf) {
	check_ptr(file);
	check_ptr(buf);
	DC::string path(file);
	auto inode_or_err = VFS::inst().resolve_path(path, _cwd, _user);
	if(inode_or_err.is_error())
		return inode_or_err.code();
	inode_or_err.value()->inode()->metadata().stat((struct stat*)buf);
	return 0;
}

int Process::sys_lstat(char *file, char *buf) {
	check_ptr(file);
	check_ptr(buf);
	DC::string path(file);
	auto inode_or_err = VFS::inst().resolve_path(path, _cwd, _user, nullptr, O_INTERNAL_RETLINK);
	if(inode_or_err.is_error())
		return inode_or_err.code();
	inode_or_err.value()->inode()->metadata().stat((struct stat*)buf);
	return 0;
}

int Process::sys_lseek(int file, off_t off, int whence) {
	if(file < 0 || file >= (int) _file_descriptors.size() || !_file_descriptors[file])
		return -EBADF;
	return _file_descriptors[file]->seek(off, whence);
}

int Process::sys_waitpid(pid_t pid, int* status, int flags) {
	//TODO: Flags
	if(status)
		check_ptr(status);
	WaitBlocker blocker(this, pid);
	block(blocker);
	if(blocker.error())
		return blocker.error();
	if(status)
		*status = blocker.exit_status();
	return blocker.waited_pid();
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
	if(sig == SIGSTOP || sig == SIGKILL || sig < 1 || sig >= 32)
		return -EINVAL;
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
	if(sig == 0)
		return 0;
	if(sig < 0 || sig >= NSIG)
		return -EINVAL;
	if(pid == _pid)
		kill(sig);
	else if(pid == 0) {
		//Kill all processes with _pgid == this->_pgid
		Process* c_proc = this->next;
		while(c_proc != this) {
			if((_user.uid == 0 || c_proc->_user.uid == _user.uid) && c_proc->_pgid == _pgid && c_proc->_pid != 1)
				c_proc->kill(sig);
			c_proc = c_proc->next;
		}
		kill(sig);
	} else if(pid == -1) {
		//kill all processes for which we have permission to kill except init
		Process* c_proc = this->next;
		while(c_proc != this) {
			if((_user.uid == 0 || c_proc->_user.uid == _user.uid) && c_proc->_pid != 1)
				c_proc->kill(sig);
			c_proc = c_proc->next;
		}
		kill(sig);
	} else if(pid < -1) {
		//Kill all processes with _pgid == -pid
		Process* c_proc = this->next;
		while(c_proc != this) {
			if((_user.uid == 0 || c_proc->_user.uid == _user.uid) && c_proc->_pgid == -pid && c_proc->_pid != 1)
				c_proc->kill(sig);
			c_proc = c_proc->next;
		}
		kill(sig);
	} else {
		//Kill process with _pid == pid
		Process* proc = TaskManager::process_for_pid(pid);
		if(!proc)
			return -ESRCH;
		if((_user.uid != 0 && proc->_user.uid != _user.uid) || proc->_pid == 1)
			return -EPERM;
		proc->kill(sig);
	}
	return 0;
}

int Process::sys_unlink(char* name) {
	check_ptr(name);
	DC::string path(name);
	auto ret = VFS::inst().unlink(path, _user, _cwd);
	if(ret.is_error())
		return ret.code();
	return 0;
}

int Process::sys_link(char* oldpath, char* newpath) {
	check_ptr(oldpath);
	check_ptr(newpath);
	DC::string oldpath_str(oldpath);
	DC::string newpath_str(newpath);
	return VFS::inst().link(oldpath_str, newpath_str, _user, _cwd).code();
}

int Process::sys_rmdir(char* name) {
	check_ptr(name);
	DC::string path(name);
	auto ret = VFS::inst().rmdir(path, _user, _cwd);
	if(ret.is_error())
		return ret.code();
	return 0;
}

int Process::sys_mkdir(char *path, mode_t mode) {
	check_ptr(path);
	DC::string strpath(path);
	mode &= 04777; //We just want the permission bits
	auto ret = VFS::inst().mkdir(strpath, mode, _user, _cwd);
	if(ret.is_error())
		return ret.code();
	return 0;
}

int Process::sys_mkdirat(int file, char *path, mode_t mode) {
	return -1;
}

int Process::sys_truncate(char* path, off_t length) {
	check_ptr(path);
	DC::string strpath(path);
	return VFS::inst().truncate(strpath, length, _user, _cwd).code();
}

int Process::sys_ftruncate(int file, off_t length) {
	return -1;
}

int Process::sys_pipe(int filedes[2]) {
	check_ptr(filedes);

	//Make the pipe
	auto pipe = DC::make_shared<Pipe>();
	pipe->add_reader();
	pipe->add_writer();

	//Make the read FD
	auto pipe_read_fd = DC::make_shared<FileDescriptor>(pipe);
	pipe_read_fd->set_owner(this);
	pipe_read_fd->set_options(O_RDONLY);
	pipe_read_fd->set_fifo_reader();
	_file_descriptors.push_back(pipe_read_fd);
	filedes[0] = (int) _file_descriptors.size() - 1;

	//Make the write FD
	auto pipe_write_fd = DC::make_shared<FileDescriptor>(pipe);
	pipe_write_fd->set_owner(this);
	pipe_write_fd->set_options(O_WRONLY);
	pipe_write_fd->set_fifo_writer();
	_file_descriptors.push_back(pipe_write_fd);
	filedes[1] = (int) _file_descriptors.size() - 1;

	return SUCCESS;
}

int Process::sys_dup(int oldfd) {
	if(oldfd < 0 || oldfd >= (int) _file_descriptors.size() || !_file_descriptors[oldfd])
		return -EBADF;
	_file_descriptors.push_back(_file_descriptors[oldfd]);
	return (int) _file_descriptors.size() - 1;
}

int Process::sys_dup2(int oldfd, int newfd) {
	if(oldfd < 0 || oldfd >= (int) _file_descriptors.size() || !_file_descriptors[oldfd])
		return -EBADF;
	if(newfd == oldfd)
		return oldfd;
	if(newfd >= _file_descriptors.size())
		_file_descriptors.resize(newfd + 1);
	_file_descriptors[newfd] = _file_descriptors[oldfd];
	return newfd;
}

int Process::sys_isatty(int file) {
	if(file < 0 || file >= (int) _file_descriptors.size() || !_file_descriptors[file])
		return -EBADF;
	return _file_descriptors[file]->file()->is_tty() ? 1 : -ENOTTY;
}

int Process::sys_symlink(char* file, char* linkname) {
	check_ptr(file);
	check_ptr(linkname);
	DC::string file_str(file);
	DC::string linkname_str(linkname);
	return VFS::inst().symlink(file_str, linkname_str, _user, _cwd).code();
}

int Process::sys_symlinkat(char* file, int dirfd, char* linkname) {
	return -1;
}

int Process::sys_readlink(char* file, char* buf, size_t bufsize) {
	if(bufsize < 0)
		return -EINVAL;
	check_ptr(file);
	check_ptr(buf);
	DC::string file_str(file);

	ssize_t ret;
	auto ret_perhaps = VFS::inst().readlink(file_str, _user, _cwd, ret);
	if(ret_perhaps.is_error())
		return ret_perhaps.code();

	auto& link_value = ret_perhaps.value();
	memcpy(buf, link_value.c_str(), min(link_value.length(), bufsize - 1));
	buf[min(bufsize - 1, link_value.length())] = '\0';

	return SUCCESS;
}

int Process::sys_readlinkat(struct readlinkat_args* args) {
	check_ptr(args);
	return -1;
}

int Process::sys_getsid(pid_t pid) {
	if(pid == 0)
		return _sid;
	auto* proc = TaskManager::process_for_pid(pid);
	if(!proc)
		return -ESRCH;
	if(proc->_sid != _sid)
		return -EPERM;
	return proc->_sid;
}

int Process::sys_setsid() {
	//Make sure there's no other processes in the group
	Process* c_proc = this->next;
	while(c_proc != this) {
		if(c_proc->_pgid == _pid)
			return -EPERM;
		c_proc = c_proc->next;
	}

	_sid = _pid;
	_pgid = _pid;
	return _sid;
}

int Process::sys_getpgid(pid_t pid) {
	if(pid == 0)
		return _pgid;
	Process* proc = TaskManager::process_for_pid(pid);
	if(!proc)
		return -ESRCH;
	return proc->_pgid;
}

int Process::sys_getpgrp() {
	return _pgid;
}

int Process::sys_setpgid(pid_t pid, pid_t new_pgid) {
	if(pid < 0)
		return -EINVAL;
	if(!new_pgid)
		new_pgid = _pid;

	//Validate specified pid
	Process* proc = pid ? TaskManager::process_for_pid(pid) : this;
	if(!proc || (proc->_pid != _pid && proc->_ppid != _pid))
		return -ESRCH; //Process doesn't exist or is not self or child
	if(proc->_ppid == _pid && proc->_sid != _sid)
		return -EPERM; //Process is a child but not in the same session
	if(proc->_pid == proc->_sid)
		return -EPERM; //Process is session leader

	//Make sure we're not switching to another session
	Process* c_proc = this->next;
	pid_t new_sid = _sid;
	while(c_proc != this) {
		if(c_proc->_pgid == new_pgid)
			new_sid = c_proc->_sid;
		c_proc = c_proc->next;
	}
	if(new_sid != _sid)
		return -EPERM;

	proc->_pgid = new_pgid;
	return SUCCESS;
}

int Process::sys_setuid(uid_t uid) {
	if(!_user.can_setuid() && uid != _user.uid && uid != _user.euid)
		return -EPERM;

	_user.uid = uid;
	_user.euid = uid;
	return SUCCESS;
}

int Process::sys_seteuid(uid_t euid) {
	if(!_user.can_setuid() && euid != _user.uid && euid != _user.euid)
		return -EPERM;

	_user.euid = euid;
	return SUCCESS;
}

uid_t Process::sys_getuid() {
	return _user.uid;
}

uid_t Process::sys_geteuid() {
	return _user.euid;
}

int Process::sys_setgid(gid_t gid) {
	if(!_user.can_setgid() && gid != _user.gid && gid != _user.egid)
		return -EPERM;

	_user.gid = gid;
	_user.egid = gid;
	return SUCCESS;
}

int Process::sys_setegid(gid_t egid) {
	if(!_user.can_setgid() && egid != _user.gid && egid != _user.egid)
		return -EPERM;

	_user.egid = egid;
	return SUCCESS;
}

gid_t Process::sys_getgid() {
	return _user.gid;
}

gid_t Process::sys_getegid() {
	return _user.egid;
}

int Process::sys_setgroups(size_t count, const gid_t* gids) {
	check_ptr(gids);
	if(count < 0)
		return -EINVAL;
	if(!_user.can_setgid())
		return -EPERM;

	if(!count) {
		_user.groups.resize(0);
		return SUCCESS;
	}

	_user.groups.resize(count);
	for(size_t i = 0; i < count; i++) _user.groups[i] = gids[i];
	return SUCCESS;
}

int Process::sys_getgroups(int count, gid_t* gids) {
	if(count < 0)
		return -EINVAL;
	if(count == 0)
		return _user.groups.size();
	if(count <  _user.groups.size())
		return -EINVAL;
	for(size_t i = 0; i <  _user.groups.size(); i++) gids[i] = _user.groups[i];
	return SUCCESS;
}

mode_t Process::sys_umask(mode_t new_mask) {
	auto ret = _umask;
	_umask = new_mask & 0777;
	return ret;
}

int Process::sys_chmod(char* file, mode_t mode) {
	check_ptr(file);
	DC::string filestr(file);
	return VFS::inst().chmod(filestr, mode, _user, _cwd).code();
}

int Process::sys_fchmod(int fd, mode_t mode) {
	return -1;
}

int Process::sys_chown(char* file, uid_t uid, gid_t gid) {
	check_ptr(file);
	DC::string filestr(file);
	return VFS::inst().chown(filestr, uid, gid, _user, _cwd).code();
}

int Process::sys_fchown(int fd, uid_t uid, gid_t gid) {
	return -1;
}

int Process::sys_lchown(char* file, uid_t uid, gid_t gid) {
	check_ptr(file);
	DC::string filestr(file);
	return VFS::inst().chown(filestr, uid, gid, _user, _cwd, O_NOFOLLOW).code();
}

int Process::sys_ioctl(int fd, unsigned request, void* argp) {
	check_ptr(argp);
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd])
		return -EBADF;
	return _file_descriptors[fd]->ioctl(request, argp);
}

void* Process::sys_memacquire(void* addr, size_t size) const {
	if(addr) {
		//We requested a specific address
		auto region = page_directory->allocate_region((size_t) addr, size, true);
		if(!region.virt)
			return (void*) -EINVAL;
		return (void*) region.virt->start;
	} else {
		//We didn't request a specific address
		auto region = page_directory->allocate_region(size, true);
		if(!region.virt)
			return (void*) -ENOMEM;
		return (void*) region.virt->start;
	}
}

int Process::sys_memrelease(void* addr, size_t size) const {
	if(!page_directory->free_region((size_t) addr, size))
		return -EINVAL;
	return SUCCESS;
}

int Process::sys_shmcreate(void* addr, size_t size, struct shm* s) {
	check_ptr(s);

	auto res = page_directory->create_shared_region((size_t) addr, size, _pid);
	if(res.is_error())
		return res.code();

	auto reg = res.value();
	s->size = reg.virt->size;
	s->ptr = (void*) reg.virt->start;
	s->id = reg.virt->shm_id;

	return SUCCESS;
}

int Process::sys_shmattach(int id, void* addr, struct shm* s) {
	check_ptr(s);

	auto res = page_directory->attach_shared_region(id, (size_t) addr, _pid);
	if(res.is_error())
		return res.code();

	auto reg = res.value();
	s->size = reg.virt->size;
	s->ptr = (void*) reg.virt->start;
	s->id = reg.phys->shm_id;

	return SUCCESS;
}

int Process::sys_shmdetach(int id) {
	return page_directory->detach_shared_region(id).code();
}

int Process::sys_shmallow(int id, pid_t pid, int perms) {
	//TODO: If PIDs end up getting recycled and we previously allowed a PID that was recycled, it could be a security issue

	if(!(perms &  (SHM_READ | SHM_WRITE)))
		return -EINVAL;
	if((perms & SHM_WRITE) && !(perms & SHM_READ))
		return -EINVAL;
	if(!TaskManager::process_for_pid(pid))
		return -EINVAL;

	return page_directory->allow_shared_region(id, _pid, pid, perms & SHM_WRITE).code();
}

int Process::sys_poll(struct pollfd* pollfd, nfds_t nfd, int timeout) {
	check_ptr(pollfd);

	//Build the list of PollBlocker::PollFDs
	DC::vector<PollBlocker::PollFD> polls;
	polls.reserve(nfd);
	for(nfds_t i = 0; i < nfd; i++) {
		auto& poll = pollfd[i];
		//Make sure the fd is valid. If not, set revents to POLLINVAL
		if(poll.fd < 0 || poll.fd >= (int) _file_descriptors.size() || !_file_descriptors[poll.fd]) {
			poll.revents = POLLINVAL;
		} else {
			poll.revents = 0;
			polls.push_back({poll.fd, _file_descriptors[poll.fd], poll.events});
		}
	}

	//Block
	PollBlocker blocker(polls, timeout);
	block(blocker);

	//Set appropriate revent
	for(nfds_t i = 0; i < nfd; i++) {
		auto& poll = pollfd[i];
		if(poll.fd == blocker.polled) {
			poll.revents = blocker.polled_revent;
			break;
		}
	}

	return SUCCESS;
}

int Process::sys_ptsname(int fd, char* buf, size_t bufsize) {
	check_ptr(buf);
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd])
		return -EBADF;
	auto file = _file_descriptors[fd];

	//Check to make sure it's a PTY Controller
	if(!file->file()->is_pty_controller())
		return -ENOTTY;

	//Make sure the name isn't too long
	auto name = ((PTYControllerDevice*) file->file().get())->pty()->name();
	if(name.length() + 1 > bufsize)
		return -ERANGE;

	//Copy the name into the buffer and return success
	strcpy(buf, name.c_str());
	return SUCCESS;
}
