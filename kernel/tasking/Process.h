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

#pragma once

#include <kernel/kstd/shared_ptr.hpp>
#include <kernel/kstd/queue.hpp>
#include "Signal.h"
#include <kernel/User.h>
#include <kernel/kstd/string.h>

class FileDescriptor;
class Blocker;
class ProcessArgs;
class TTYDevice;
class Thread;
class PageDirectory;
class LinkedInode;

namespace ELF {struct elf32_header;};

template<typename T>
class UserspacePointer;

#define SHM_READ 0x1u
#define SHM_WRITE 0x2u
#define SHM_SHARE 0x4u

class Process {
public:
	enum State {
		ALIVE = 0, //The process has not exited yet
		ZOMBIE = 1, //The process has exited and needs to be reaped
		DEAD = 2 //The process has been reaped and needs to be removed from the process table
	};

	~Process();

	//Construction
	static Process* create_kernel(const kstd::string& name, void (*func)());
	static ResultRet<Process*> create_user(const kstd::string& executable_loc, User& file_open_user, ProcessArgs* args, pid_t parent);

	//Process Info
	pid_t pid();
	pid_t pgid();
	pid_t ppid();
	void set_ppid(pid_t ppid);
	pid_t sid();
	User user();
	kstd::string name();
	kstd::string exe();
	kstd::shared_ptr<LinkedInode> cwd();
	void set_tty(kstd::shared_ptr<TTYDevice> tty);
	State state();
	int main_thread_state();
	int exit_status();
	bool is_kernel_mode();

	//Threads
	kstd::shared_ptr<Thread>& main_thread();
	tid_t last_active_thread();
	void set_last_active_thread(tid_t tid);
	const kstd::vector<kstd::shared_ptr<Thread>>& threads();

	//Signals and death
	void kill(int signal);
	void reap();
	void free_resources();
	bool handle_pending_signal();
	bool has_pending_signals();

	//Memory
	PageDirectory* page_directory();

	//Syscalls
	void check_ptr(const void* ptr);
	void sys_exit(int status);
	ssize_t sys_read(int fd, uint8_t* buf, size_t count);
	ssize_t sys_write(int fd, uint8_t* buf, size_t count);
	pid_t sys_fork(Registers& regs);
	int exec(const kstd::string& filename, ProcessArgs* args);
	int sys_execve(UserspacePointer<char> filename, UserspacePointer<char*> argv, UserspacePointer<char*> envp);
	int sys_execvp(UserspacePointer<char> filename, UserspacePointer<char*> argv);
	int sys_open(UserspacePointer<char> filename, int options, int mode);
	int sys_close(int file);
	int sys_chdir(UserspacePointer<char> path);
	int sys_getcwd(UserspacePointer<char> buf, size_t length);
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
	int sys_pipe(int filedes[2], int options);
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
	int sys_poll(UserspacePointer<pollfd> pollfd, nfds_t nfd, int timeout);
	int sys_ptsname(int fd, char* buf, size_t bufsize);
	int sys_sleep(timespec* time, timespec* remainder);
	int sys_threadcreate(void* (*entry_func)(void* (*)(void*), void*), void* (*thread_func)(void*), void* arg);
	int sys_gettid();
	int sys_threadjoin(tid_t tid, void** retp);
	int sys_threadexit(void* return_value);

private:
	friend class Thread;
	Process(const kstd::string& name, size_t entry_point, bool kernel, ProcessArgs* args, pid_t parent);
	Process(Process* to_fork, Registers& regs);

	void alert_thread_died();

	//Identifying info and state
	kstd::string _name = "";
	kstd::string _exe = "";
	pid_t _pid = 0;
	pid_t _ppid = 0;
	pid_t _sid = 0;
	pid_t _pgid = 0;
	kstd::shared_ptr<TTYDevice> _tty;
	User _user;
	mode_t _umask = 022;
	int _exit_status = 0;
	bool _freed_resources = false;
	State _state;
	bool _kernel_mode = false;

	//Memory
	kstd::shared_ptr<PageDirectory> _page_directory;

	//Files & Pipes
	kstd::vector<kstd::shared_ptr<FileDescriptor>> _file_descriptors;
	kstd::shared_ptr<LinkedInode> _cwd;

	//Blocking stuff
	SpinLock _lock;

	//Signals
	Signal::SigAction signal_actions[32] = {{Signal::SigAction()}};
	kstd::queue<int> pending_signals;

	//Threads
	kstd::vector<kstd::shared_ptr<Thread>> _threads;
	tid_t _cur_tid = 1;
	tid_t _last_active_thread = 1;

	Process* _self_ptr;
};


