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

#ifndef DUCKOS_LIBC_SYSCALL_H
#define DUCKOS_LIBC_SYSCALL_H

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
#define SYS_DUP 31
#define SYS_DUP2 32
#define SYS_LSTAT 33
#define SYS_SYMLINK 34
#define SYS_SYMLINKAT 35
#define SYS_READLINK 36
#define SYS_READLINKAT 37
#define SYS_GETSID 38
#define SYS_SETSID 39
#define SYS_GETPGID 40
#define SYS_GETPGRP 41
#define SYS_SETPGID 42
#define SYS_SETUID 43
#define SYS_SETEUID 44
#define SYS_GETUID 45
#define SYS_GETEUID 46
#define SYS_SETGID 47
#define SYS_SETEGID 48
#define SYS_GETGID 49
#define SYS_GETEGID 50
#define SYS_SETGROUPS 51
#define SYS_GETGROUPS 52
#define SYS_UMASK 53
#define SYS_CHMOD 54
#define SYS_FCHMOD 55
#define SYS_CHOWN 56
#define SYS_FCHOWN 57
#define SYS_LCHOWN 58
#define SYS_INTERNAL_ALLOC 59
#define SYS_GETPPID 60

int syscall(int call);
int syscall2(int call, int b);
int syscall3(int call, int b, int c);
int syscall4(int call, int b, int c, int d);

#endif //DUCKOS_LIBC_SYSCALL_H