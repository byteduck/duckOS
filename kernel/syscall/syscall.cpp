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

#include <kernel/kstd/kstddef.h>
#include "syscall.h"
#include <kernel/kstd/kstdio.h>
#include <kernel/tasking/TaskManager.h>
#include "kernel/memory/SafePointer.h"

void syscall_handler(ThreadRegisters& regs){
	TrapFrame frame { nullptr, TrapFrame::Syscall, &regs };
	TaskManager::current_thread()->enter_trap_frame(&frame);
	TaskManager::current_thread()->enter_syscall();
	regs.gp.eax = handle_syscall(regs, regs.gp.eax, regs.gp.ebx, regs.gp.ecx, regs.gp.edx);
	TaskManager::current_thread()->leave_syscall();
	TaskManager::current_thread()->exit_trap_frame();
}

int handle_syscall(ThreadRegisters& regs, size_t call, size_t arg1, size_t arg2, size_t arg3) {
	auto cur_proc = TaskManager::current_thread()->process();
	switch(call) {
		case SYS_EXIT:
			TaskManager::current_thread()->leave_syscall();
			cur_proc->sys_exit(arg1);
			return 0;
		case SYS_FORK:
			return cur_proc->sys_fork(regs);
		case SYS_READ:
			return cur_proc->sys_read((int)arg1, (uint8_t*)arg2, (size_t)arg3);
		case SYS_WRITE:
			return cur_proc->sys_write((int)arg1, (uint8_t*)arg2, (size_t)arg3);
		case SYS_EXECVE:
			return cur_proc->sys_execve((char*)arg1, (char**)arg2, (char**)arg3);
		case SYS_OPEN:
			return cur_proc->sys_open((char*)arg1, (int)arg2, (int)arg3);
		case SYS_CLOSE:
			return cur_proc->sys_close((int)arg1);
		case SYS_FSTAT:
			return cur_proc->sys_fstat((int)arg1, (struct stat*)arg2);
		case SYS_STAT:
			return cur_proc->sys_stat((char*)arg1, (struct stat*)arg2);
		case SYS_LSEEK:
			return cur_proc->sys_lseek((int)arg1, (off_t)arg2, (int)arg3);
		case SYS_GETPID:
			return cur_proc->pid();
		case SYS_READDIR:
			return cur_proc->sys_readdir((int)arg1, (char*)arg2, (size_t)arg3);
		case SYS_CHDIR:
			return cur_proc->sys_chdir((char*)arg1);
		case SYS_GETCWD:
			return cur_proc->sys_getcwd((char*)arg1, (size_t)arg2);
		case SYS_WAITPID:
			return cur_proc->sys_waitpid((pid_t)arg1, (int*)arg2, (int)arg3);
		case SYS_GETTIMEOFDAY:
			return cur_proc->sys_gettimeofday((struct timeval*)arg1, (void**)arg2);
		case SYS_SIGACTION:
			return cur_proc->sys_sigaction((int)arg1, (struct sigaction*)arg2, (struct sigaction*)arg3);
		case SYS_KILL:
			return cur_proc->sys_kill((pid_t)arg1, (int)arg2);
		case SYS_UNLINK:
			return cur_proc->sys_unlink((char*)arg1);
		case SYS_LINK:
			return cur_proc->sys_link((char*)arg1, (char*)arg2);
		case SYS_RMDIR:
			return cur_proc->sys_rmdir((char*)arg1);
		case SYS_MKDIR:
			return cur_proc->sys_mkdir((char*)arg1, (mode_t)arg2);
		case SYS_MKDIRAT:
			return cur_proc->sys_mkdirat((int)arg1, (char*)arg2, (mode_t)arg3);
		case SYS_TRUNCATE:
			return cur_proc->sys_truncate((char*)arg1, (off_t)arg2);
		case SYS_FTRUNCATE:
			return cur_proc->sys_ftruncate((int)arg1, (off_t)arg2);
		case SYS_PIPE:
			return cur_proc->sys_pipe((int*)arg1, (int)arg2);
		case SYS_DUP:
			return cur_proc->sys_dup((int)arg1);
		case SYS_DUP2:
			return cur_proc->sys_dup2((int)arg1, (int)arg2);
		case SYS_ISATTY:
			return cur_proc->sys_isatty((int)arg1);
		case SYS_LSTAT:
			return cur_proc->sys_lstat((char*)arg1, (struct stat*)arg2);
		case SYS_SYMLINK:
			return cur_proc->sys_symlink((char*)arg1, (char*)arg2);
		case SYS_SYMLINKAT:
			return cur_proc->sys_symlinkat((char*)arg1, (int)arg2, (char*)arg3);
		case SYS_READLINK:
			return cur_proc->sys_readlink((char*)arg1, (char*)arg2, (size_t)arg3);
		case SYS_READLINKAT:
			return cur_proc->sys_readlinkat((struct readlinkat_args*) arg1);
		case SYS_GETSID:
			return cur_proc->sys_getsid((pid_t)arg1);
		case SYS_SETSID:
			return cur_proc->sys_setsid();
		case SYS_GETPGID:
			return cur_proc->sys_getpgid((pid_t)arg1);
		case SYS_GETPGRP:
			return cur_proc->sys_getpgrp();
		case SYS_SETPGID:
			return cur_proc->sys_setpgid((pid_t)arg1, (pid_t)arg2);
		case SYS_SETUID:
			return cur_proc->sys_setuid((uid_t)arg1);
		case SYS_SETEUID:
			return cur_proc->sys_seteuid((uid_t)arg1);
		case SYS_GETUID:
			return cur_proc->sys_getuid();
		case SYS_GETEUID:
			return cur_proc->sys_geteuid();
		case SYS_SETGID:
			return cur_proc->sys_setgid((gid_t)arg1);
		case SYS_SETEGID:
			return cur_proc->sys_setegid((gid_t)arg1);
		case SYS_GETGID:
			return cur_proc->sys_getgid();
		case SYS_GETEGID:
			return cur_proc->sys_getegid();
		case SYS_SETGROUPS:
			return cur_proc->sys_setgroups((size_t)arg1, (gid_t*)arg2);
		case SYS_GETGROUPS:
			return cur_proc->sys_getgroups((int)arg1, (gid_t*)arg2);
		case SYS_UMASK:
			return cur_proc->sys_umask((mode_t)arg1);
		case SYS_CHMOD:
			return cur_proc->sys_chmod((char*)arg1, (mode_t)arg2);
		case SYS_FCHMOD:
			return cur_proc->sys_fchmod((int)arg1, (mode_t)arg2);
		case SYS_CHOWN:
			return cur_proc->sys_chown((char*)arg1, (uid_t)arg2, (gid_t)arg3);
		case SYS_FCHOWN:
			return cur_proc->sys_fchown((int)arg1, (uid_t)arg2, (gid_t)arg3);
		case SYS_LCHOWN:
			return cur_proc->sys_lchown((char*)arg1, (uid_t)arg2, (gid_t)arg3);
		case SYS_MMAP:
			return -cur_proc->sys_mmap((struct mmap_args*) arg1).code();
		case SYS_MUNMAP:
			return cur_proc->sys_munmap((void*) arg1, (size_t) arg2);
		case SYS_IOCTL:
			return cur_proc->sys_ioctl((int)arg1, (unsigned) arg2, (void**) arg3);
		case SYS_GETPPID:
			return cur_proc->ppid();
		case SYS_SHMCREATE:
			return cur_proc->sys_shmcreate((shmcreate_args*) arg1);
		case SYS_SHMATTACH:
			return cur_proc->sys_shmattach((int) arg1, (void*) arg2, (struct shm*) arg3);
		case SYS_SHMDETACH:
			return cur_proc->sys_shmdetach((int)arg1);
		case SYS_SHMALLOW:
			return cur_proc->sys_shmallow((int) arg1, (pid_t) arg2, (int) arg3);
		case SYS_POLL:
			return cur_proc->sys_poll((struct pollfd*) arg1, (nfds_t) arg2, (int) arg3);
		case SYS_PTSNAME:
			return cur_proc->sys_ptsname(arg1, (char*) arg2, (int) arg3);
		case SYS_SLEEP:
			return cur_proc->sys_sleep((timespec*) arg1, (timespec*) arg2);
		case SYS_THREADCREATE:
			return cur_proc->sys_threadcreate((void* (*)(void* (*)(void*), void*)) arg1, (void* (*)(void*)) arg2, (void*) arg3);
		case SYS_GETTID:
			return cur_proc->sys_gettid();
		case SYS_THREADJOIN:
			return cur_proc->sys_threadjoin((tid_t) arg1, (void**) arg2);
		case SYS_THREADEXIT:
			return cur_proc->sys_threadexit((void*) arg1);
		case SYS_ISCOMPUTERON:
			return true;
		case SYS_ACCESS:
			return cur_proc->sys_access((char*) arg1, (int) arg2);
		case SYS_MPROTECT:
			return cur_proc->sys_mprotect((void*) arg1, (size_t) arg2, arg3);
		case SYS_UNAME:
			return cur_proc->sys_uname((struct utsname*) arg1);
		case SYS_PTRACE:
			return cur_proc->sys_ptrace((struct ptrace_args*) arg1);
		case SYS_SOCKET:
			return cur_proc->sys_socket(arg1, arg2, arg3);
		case SYS_BIND:
			return cur_proc->sys_bind(arg1, (struct sockaddr*) arg2, arg3);
		case SYS_SETSOCKOPT:
			return cur_proc->sys_setsockopt((struct setsockopt_args*) arg1);
		case SYS_GETSOCKOPT:
			return cur_proc->sys_getsockopt((struct getsockopt_args*) arg1);
		case SYS_SENDMSG:
			return cur_proc->sys_sendmsg(arg1, (struct msghdr*) arg2, arg3);
		case SYS_RECVMSG:
			return cur_proc->sys_recvmsg(arg1, (struct msghdr*) arg2, arg3);
		case SYS_GETIFADDRS:
			return cur_proc->sys_getifaddrs((struct ifaddrs*) arg1, (size_t) arg2);
		case SYS_CONNECT:
			return cur_proc->sys_connect(arg1, (struct sockaddr*) arg2, arg3);
		case SYS_LISTEN:
			return cur_proc->sys_listen(arg1, arg2);
		case SYS_SHUTDOWN:
			return cur_proc->sys_shutdown(arg1, arg2);
		case SYS_ACCEPT:
			return cur_proc->sys_accept(arg1, (struct sockaddr*) arg2, (uint32_t*) arg3);
		case SYS_FUTEX:
			return cur_proc->sys_futex((int*) arg1, arg2);
		case SYS_YIELD:
			TaskManager::yield();
			return 0;

		//TODO: Implement these syscalls
		case SYS_TIMES:
			return -1;

		default:
#ifdef DEBUG
			printf("UNKNOWN_SYSCALL(%d, %d, %d, %d)\n", call, arg1, arg2, arg3);
#endif
			return 0;

	}
}
