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

#ifndef SYSCALL_H
#define SYSCALL_H

#define SYS_EXIT 1
#define SYS_FORK 2
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_SBRK 5
#define SYS_EXECVE 6
#define SYS_OPEN 7
#define SYS_CLOSE 8
#define SYS_FSTAT 9
#define SYS_STAT 10
#define SYS_LSEEK 11
#define SYS_KILL 12
#define SYS_GETPID 13
#define SYS_TIMES 14
#define SYS_UNLINK 15
#define SYS_GETTIMEOFDAY 16
#define SYS_SIGACTION 17
#define SYS_ISATTY 18
#define SYS_LINK 19
#define SYS_WAITPID 20
#define SYS_READDIR 21
#define SYS_CHDIR 22
#define SYS_GETCWD 23
#define SYS_EXECVP 24
#define SYS_RMDIR 25
#define SYS_MKDIR 26
#define SYS_MKDIRAT 27
#define SYS_TRUNCATE 28
#define SYS_FTRUNCATE 29
#define SYS_PIPE 30

extern "C" void syscall_handler(Registers& regs);
int handle_syscall(Registers& regs, uint32_t call, uint32_t arg1, uint32_t arg2, uint32_t arg3);

#endif