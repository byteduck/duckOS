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

#include <kernel/kstd/kstddef.h>
#include <kernel/kstd/string.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/tasking/ELF.h>
#include <kernel/kstd/shared_ptr.hpp>
#include <kernel/filesystem/LinkedInode.h>
#include <kernel/tasking/TSS.h>
#include "ProcessArgs.h"
#include <kernel/memory/PageDirectory.h>
#include <kernel/kstd/queue.hpp>
#include "Signal.h"
#include "Blocker.h"
#include <kernel/filesystem/Pipe.h>
#include <kernel/interrupt/syscall.h>

#define PROCESS_STACK_SIZE 1048576 //1024KiB
#define PROCESS_KERNEL_STACK_SIZE 4096 //4KiB
#define PROCESS_ALIVE 0
#define PROCESS_ZOMBIE 1
#define PROCESS_DEAD 2
#define PROCESS_BLOCKED 3

class TaskBlockQueue;
class FileDescriptor;
class Blocker;

namespace ELF {struct elf32_header;};

#define SHM_READ 0x1u
#define SHM_WRITE 0x2u

struct shm {
	void* ptr;
	size_t size;
	int id;
};

struct pollfd {
public:
	int fd;
	short events;
	short revents;
};

typedef size_t nfds_t;

extern const char* PROC_STATUS_NAMES[];

class TTYDevice;
class Process {
public:
	~Process();

	static Process* create_kernel(const kstd::string& name, void (*func)());
	static ResultRet<Process*> create_user(const kstd::string& executable_loc, User& file_open_user, ProcessArgs* args, pid_t parent);

	pid_t pid();
	pid_t pgid();
	pid_t ppid();
	void set_ppid(pid_t ppid);
	pid_t sid();
	User user();
	kstd::string name();
	kstd::string exe();
	kstd::shared_ptr<LinkedInode> cwd();
	void set_tty(TTYDevice* tty);

	int exit_status();
	void kill(int signal);
	void reap();
	void free_resources();
	void handle_pagefault(Registers *regs);
	void* kernel_stack_top();

	bool handle_pending_signal();
	bool has_pending_signals();
	void call_signal_handler(int signal);
	bool& in_signal_handler();
	bool& ready_to_handle_signal();
	bool& just_finished_signal();
	void* signal_stack_top();

	void block(Blocker& blocker);
	void unblock();
	bool is_blocked();
	bool should_unblock();

	//Syscalls
	void check_ptr(const void* ptr);
	void sys_exit(int status);
	ssize_t sys_read(int fd, uint8_t* buf, size_t count);
	ssize_t sys_write(int fd, uint8_t* buf, size_t count);
	pid_t sys_fork(Registers& regs);
	int exec(const kstd::string& filename, ProcessArgs* args);
	int sys_execve(char *filename, char **argv, char **envp);
	int sys_execvp(char *filename, char **argv);
	int sys_open(char *filename, int options, int mode);
	int sys_close(int file);
	int sys_chdir(char *path);
	int sys_getcwd(char *buf, size_t length);
	int sys_readdir(int file, char* buf, size_t len);
	int sys_fstat(int file, char* buf);
	int sys_stat(char* file, char* buf);
	int sys_lstat(char* file, char* buf);
	int sys_lseek(int file, off_t off, int whence);
	int sys_waitpid(pid_t pid, int* status, int flags);
	int sys_gettimeofday(timespec *t, void *z);
	int sys_sigaction(int sig, struct sigaction *new_action, struct sigaction *old_action);
	int sys_kill(pid_t pid, int sig);
	int sys_unlink(char* name);
	int sys_link(char* oldpath, char* newpath);
	int sys_rmdir(char* name);
	int sys_mkdir(char *path, mode_t mode);
	int sys_mkdirat(int fd, char *path, mode_t mode);
	int sys_truncate(char* path, off_t length);
	int sys_ftruncate(int fd, off_t length);
	int sys_pipe(int filedes[2]);
	int sys_dup(int oldfd);
	int sys_dup2(int oldfd, int newfd);
	int sys_isatty(int fd);
	int sys_symlink(char* file, char* linkname);
	int sys_symlinkat(char* file, int dirfd, char* linkname);
	int sys_readlink(char* file, char* buf, size_t bufsize);
	int sys_readlinkat(struct readlinkat_args* args);
	int sys_getsid(pid_t pid);
	int sys_setsid();
	int sys_getpgid(pid_t pid);
	int sys_getpgrp();
	int sys_setpgid(pid_t pid, pid_t new_pgid);
	int sys_setuid(uid_t uid);
	int sys_seteuid(uid_t euid);
	uid_t sys_getuid();
	uid_t sys_geteuid();
	int sys_setgid(gid_t gid);
	int sys_setegid(gid_t egid);
	gid_t sys_getgid();
	gid_t sys_getegid();
	int sys_setgroups(size_t count, const gid_t* gids);
	int sys_getgroups(int count, gid_t* gids);
	mode_t sys_umask(mode_t new_mask);
	int sys_chmod(char* file, mode_t mode);
	int sys_fchmod(int fd, mode_t mode);
	int sys_chown(char* file, uid_t uid, gid_t gid);
	int sys_fchown(int fd, uid_t uid, gid_t gid);
	int sys_lchown(char* file, uid_t uid, gid_t gid);
	int sys_ioctl(int fd, unsigned request, void* argp);
	void* sys_memacquire(void* addr, size_t size) const;
	int sys_memrelease(void* addr, size_t size) const;
	int sys_shmcreate(void* addr, size_t size, struct shm* s);
	int sys_shmattach(int id, void* addr, struct shm* s);
	int sys_shmdetach(int id);
	int sys_shmallow(int id, pid_t pid, int perms);
	int sys_poll(struct pollfd* pollfd, nfds_t nfd, int timeout);
	int sys_ptsname(int fd, char* buf, size_t bufsize);
	int sys_sleep(timespec* time, timespec* remainder);

	uint32_t state = 0;
	Process *next = nullptr, *prev = nullptr;
	size_t page_directory_loc;
	Registers registers = {};
	Registers signal_registers = {};
	bool kernel = false;
	bool in_syscall = false;
	uint8_t ring;
	uint8_t quantum;
	PageDirectory* page_directory;

private:
	Process(const kstd::string& name, size_t entry_point, bool kernel, ProcessArgs* args, pid_t parent);
	Process(Process* to_fork, Registers& regs);

	void setup_stack(uint32_t*& kernel_stack, const uint32_t* user_stack, Registers& registers);

	//Identifying info and state
	kstd::string _name = "";
	kstd::string _exe = "";
	pid_t _pid = 0;
	pid_t _ppid = 0;
	pid_t _sid = 0;
	pid_t _pgid = 0;
	TTYDevice* _tty = nullptr;
	User _user;
	mode_t _umask = 022;
	int _exit_status = 0;
	bool _freed_resources = false;

	//Kernel stack
	void* _kernel_stack_base;
	size_t _kernel_stack_size;
	size_t _stack_size;

	//Files & Pipes
	kstd::vector<kstd::shared_ptr<FileDescriptor>> _file_descriptors;
	kstd::shared_ptr<LinkedInode> _cwd;

	//Blocking stuff
	Blocker* _blocker = nullptr;
	SpinLock _lock;

	//Signals
	Signal::SigAction signal_actions[32] = {{Signal::SigAction()}};
	kstd::queue<int> pending_signals;
	bool _in_signal = false;
	bool _ready_to_handle_signal = false;
	bool _just_finished_signal = false;
	size_t _signal_stack_top = 0;
	LinkedMemoryRegion _sighandler_ustack_region;
	LinkedMemoryRegion _sighandler_kstack_region;
};


#endif //DUCKOS_PROCESS_H
