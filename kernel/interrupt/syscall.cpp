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

void syscall_handler(Registers regs){
	regs.eax = handle_syscall(regs, regs.eax, regs.ebx, regs.ecx, regs.edx);
}

int handle_syscall(Registers& regs, uint32_t call, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
	switch(call) {
		case SYS_EXIT:
			TaskManager::current_process()->kill();
			return 0;
		case SYS_FORK:
			return TaskManager::current_process()->sys_fork(regs);
		case SYS_READ:
			return TaskManager::current_process()->sys_read((int)arg1, (uint8_t*)arg2, (size_t)arg3);
		case SYS_WRITE:
			return TaskManager::current_process()->sys_write((int)arg1, (uint8_t*)arg2, (size_t)arg3);
		case SYS_SBRK:
			return TaskManager::current_process()->sys_sbrk((int)arg1);
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

		//TODO: Implement these syscalls
		case SYS_KILL:
		case SYS_TIMES:
		case SYS_UNLINK:
		case SYS_SIGNAL:
		case SYS_ISATTY:
		case SYS_LINK:
			return -1;

		default:
#ifdef DEBUG
			printf("UNKNOWN_SYSCALL(%d, %d, %d, %d)\n", call, arg1, arg2, arg3);
			return 0;
#endif
	}
}