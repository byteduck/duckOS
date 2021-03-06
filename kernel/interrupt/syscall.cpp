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

#include <kernel/kstddef.h>
#include <kernel/interrupt/syscall.h>
#include <kernel/kstdio.h>
#include <kernel/tasking/TaskManager.h>

void syscall_handler(Registers& regs){
	Process* proc = TaskManager::current_process();
	proc->in_syscall = true;
	regs.eax = handle_syscall(regs, regs.eax, regs.ebx, regs.ecx, regs.edx);
	proc->in_syscall = false;
}

int handle_syscall(Registers& regs, uint32_t call, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
	switch(call) {
		case SYS_EXIT:
			TaskManager::current_process()->sys_exit(arg1);
			TaskManager::current_process()->kill(SIGKILL);
			return 0;
		case SYS_FORK:
			return TaskManager::current_process()->sys_fork(regs);
		case SYS_READ:
			return TaskManager::current_process()->sys_read((int)arg1, (uint8_t*)arg2, (size_t)arg3);
		case SYS_WRITE:
			return TaskManager::current_process()->sys_write((int)arg1, (uint8_t*)arg2, (size_t)arg3);
		case SYS_EXECVE:
			return TaskManager::current_process()->sys_execve((char*)arg1, (char**)arg2, (char**)arg3);
		case SYS_OPEN:
			return TaskManager::current_process()->sys_open((char*)arg1, (int)arg2, (int)arg3);
		case SYS_CLOSE:
			return TaskManager::current_process()->sys_close((int)arg1);
		case SYS_FSTAT:
			return TaskManager::current_process()->sys_fstat((int)arg1, (char*)arg2);
		case SYS_STAT:
			return TaskManager::current_process()->sys_stat((char*)arg1, (char*)arg2);
		case SYS_LSEEK:
			return TaskManager::current_process()->sys_lseek((int)arg1, (off_t)arg2, (int)arg3);
		case SYS_GETPID:
			return TaskManager::current_process()->pid();
		case SYS_READDIR:
			return TaskManager::current_process()->sys_readdir((int)arg1, (char*)arg2, (size_t)arg3);
		case SYS_CHDIR:
			return TaskManager::current_process()->sys_chdir((char*)arg1);
		case SYS_GETCWD:
			return TaskManager::current_process()->sys_getcwd((char*)arg1, (size_t)arg2);
		case SYS_WAITPID:
			return TaskManager::current_process()->sys_waitpid((pid_t)arg1, (int*)arg2, (int)arg3);
		case SYS_EXECVP:
			return TaskManager::current_process()->sys_execvp((char*)arg1, (char**)arg2);
		case SYS_GETTIMEOFDAY:
			return TaskManager::current_process()->sys_gettimeofday((struct timespec*)arg1, (void*)arg2);
		case SYS_SIGACTION:
			return TaskManager::current_process()->sys_sigaction((int)arg1, (struct sigaction*)arg2, (struct sigaction*)arg3);
		case SYS_KILL:
			return TaskManager::current_process()->sys_kill((pid_t)arg1, (int)arg2);
		case SYS_UNLINK:
			return TaskManager::current_process()->sys_unlink((char*)arg1);
		case SYS_LINK:
			return TaskManager::current_process()->sys_link((char*)arg1, (char*)arg2);
		case SYS_RMDIR:
			return TaskManager::current_process()->sys_rmdir((char*)arg1);
		case SYS_MKDIR:
			return TaskManager::current_process()->sys_mkdir((char*)arg1, (mode_t)arg2);
		case SYS_MKDIRAT:
			return TaskManager::current_process()->sys_mkdirat((int)arg1, (char*)arg2, (mode_t)arg3);
		case SYS_TRUNCATE:
			return TaskManager::current_process()->sys_truncate((char*)arg1, (off_t)arg2);
		case SYS_FTRUNCATE:
			return TaskManager::current_process()->sys_ftruncate((int)arg1, (off_t)arg2);
		case SYS_PIPE:
			return TaskManager::current_process()->sys_pipe((int*)arg1);
		case SYS_DUP:
			return TaskManager::current_process()->sys_dup((int)arg1);
		case SYS_DUP2:
			return TaskManager::current_process()->sys_dup2((int)arg1, (int)arg2);
		case SYS_ISATTY:
			return TaskManager::current_process()->sys_isatty((int)arg1);
		case SYS_LSTAT:
			return TaskManager::current_process()->sys_lstat((char*)arg1, (char*)arg2);
		case SYS_SYMLINK:
			return TaskManager::current_process()->sys_symlink((char*)arg1, (char*)arg2);
		case SYS_SYMLINKAT:
			return TaskManager::current_process()->sys_symlinkat((char*)arg1, (int)arg2, (char*)arg3);
		case SYS_READLINK:
			return TaskManager::current_process()->sys_readlink((char*)arg1, (char*)arg2, (size_t)arg3);
		case SYS_READLINKAT:
			return TaskManager::current_process()->sys_readlinkat((struct readlinkat_args*) arg1);
		case SYS_GETSID:
			return TaskManager::current_process()->sys_getsid((pid_t)arg1);
		case SYS_SETSID:
			return TaskManager::current_process()->sys_setsid();
		case SYS_GETPGID:
			return TaskManager::current_process()->sys_getpgid((pid_t)arg1);
		case SYS_GETPGRP:
			return TaskManager::current_process()->sys_getpgrp();
		case SYS_SETPGID:
			return TaskManager::current_process()->sys_setpgid((pid_t)arg1, (pid_t)arg2);
		case SYS_SETUID:
			return TaskManager::current_process()->sys_setuid((uid_t)arg1);
		case SYS_SETEUID:
			return TaskManager::current_process()->sys_seteuid((uid_t)arg1);
		case SYS_GETUID:
			return TaskManager::current_process()->sys_getuid();
		case SYS_GETEUID:
			return TaskManager::current_process()->sys_geteuid();
		case SYS_SETGID:
			return TaskManager::current_process()->sys_setgid((gid_t)arg1);
		case SYS_SETEGID:
			return TaskManager::current_process()->sys_setegid((gid_t)arg1);
		case SYS_GETGID:
			return TaskManager::current_process()->sys_getgid();
		case SYS_GETEGID:
			return TaskManager::current_process()->sys_getegid();
		case SYS_SETGROUPS:
			return TaskManager::current_process()->sys_setgroups((size_t)arg1, (const gid_t*)arg2);
		case SYS_GETGROUPS:
			return TaskManager::current_process()->sys_getgroups((int)arg1, (gid_t*)arg2);
		case SYS_UMASK:
			return TaskManager::current_process()->sys_umask((mode_t)arg1);
		case SYS_CHMOD:
			return TaskManager::current_process()->sys_chmod((char*)arg1, (mode_t)arg2);
		case SYS_FCHMOD:
			return TaskManager::current_process()->sys_fchmod((int)arg1, (mode_t)arg2);
		case SYS_CHOWN:
			return TaskManager::current_process()->sys_chown((char*)arg1, (uid_t)arg2, (gid_t)arg3);
		case SYS_FCHOWN:
			return TaskManager::current_process()->sys_fchown((int)arg1, (uid_t)arg2, (gid_t)arg3);
		case SYS_LCHOWN:
			return TaskManager::current_process()->sys_lchown((char*)arg1, (uid_t)arg2, (gid_t)arg3);
		case SYS_MEMACQUIRE:
			return (int) TaskManager::current_process()->sys_memacquire((void*) arg1, (size_t) arg2);
		case SYS_MEMRELEASE:
			return TaskManager::current_process()->sys_memrelease((void*) arg1, (size_t) arg2);
		case SYS_IOCTL:
			return TaskManager::current_process()->sys_ioctl((int)arg1, (unsigned) arg2, (void*) arg3);
		case SYS_GETPPID:
			return TaskManager::current_process()->ppid();
		case SYS_SHMCREATE:
			return TaskManager::current_process()->sys_shmcreate((void*) arg1, (size_t) arg2, (struct shm*) arg3);
		case SYS_SHMATTACH:
			return TaskManager::current_process()->sys_shmattach((int) arg1, (void*) arg2, (struct shm*) arg3);
		case SYS_SHMDETACH:
			return TaskManager::current_process()->sys_shmdetach((int)arg1);
		case SYS_SHMALLOW:
			return TaskManager::current_process()->sys_shmallow((int) arg1, (pid_t) arg2, (int) arg3);
		case SYS_POLL:
			return TaskManager::current_process()->sys_poll((struct pollfd*) arg1, (nfds_t) arg2, (int) arg3);
		case SYS_PTSNAME:
			return TaskManager::current_process()->sys_ptsname(arg1, (char*) arg2, (int) arg3);

		//TODO: Implement these syscalls
		case SYS_TIMES:
			return -1;

		default:
#ifdef DEBUG
			printf("UNKNOWN_SYSCALL(%d, %d, %d, %d)\n", call, arg1, arg2, arg3);
			return 0;
#else
			;
#endif

	}
}
