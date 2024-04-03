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

#ifndef DUCKOS_UNISTD_H
#define DUCKOS_UNISTD_H

#include <sys/cdefs.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <kernel/api/unistd.h>

__DECL_BEGIN

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#define F_OK 1
#define R_OK 2
#define W_OK 3
#define X_OK 4

#define DEFAULT_PATH "/usr/local/bin:/usr/bin:/bin"

extern char** environ;

pid_t fork();
int execv(const char* path, char* const argv[]);
int execve(const char* filename, char* const argv[], char* const envp[]);
int execvpe(const char* filename, char* const argv[], char* const envp[]);
int execvp(const char* filename, char* const argv[]);
int execl(const char* filename, const char* arg, ...);
int execlp(const char* filename, const char* arg, ...);

pid_t getpid();
pid_t getppid();
pid_t getsid(pid_t pid);
pid_t setsid();
int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
pid_t getpgrp();
uid_t getuid();
uid_t geteuid();
gid_t getgid();
gid_t getegid();
int getgroups(int ngroups, gid_t list[]);
int setgroups(size_t ngroups, const gid_t* list);
int setuid(uid_t uid);
int setgid(gid_t gid);

ssize_t read(int fd, void* buf, size_t count);
ssize_t pread(int fd, void* buf, size_t count, off_t offset);
ssize_t write(int fd, const void* buf, size_t count);
ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset);
off_t lseek(int fd, off_t off, int whence);
int fchown(int fd, uid_t uid, gid_t gid);
int ftruncate(int fd, off_t length);
int close(int fd);
int isatty(int fd);

ssize_t readlink(const char* path, char* buf, size_t size);
ssize_t readlinkat(int fd, const char* path, char* buf, size_t size);
int link(const char* oldpath, const char* newpath);
int unlink(const char* pathname);
int symlink(const char* target, const char* linkname);
int rmdir(const char* pathname);
int chown(const char* pathname, uid_t uid, gid_t gid);
int truncate(const char* path, off_t length);
int access(const char* path, int how);

int dup(int fd);
int dup2(int old_fd, int new_fd);
int pipe(int pipefd[2]);
int pipe2(int pipefd[2], int flags);

#define _PC_NAME_MAX 0
#define _PC_PATH_MAX 1
#define _PC_VDISABLE 2
#define _PC_LINK_MAX 3
#define _POSIX_VDISABLE '\0'
long pathconf(const char* path, int name);
long fpathconf(int fd, int name);

int chdir(const char* pathname);
int fchdir(int fd);
char* getcwd(char* buf, size_t size);
char* getwd(char* buf);

int sleep(unsigned secs);
int usleep(useconds_t usec);

pid_t tcgetpgrp(int fd);
int tcsetpgrp(int fd, pid_t pgid);

unsigned int alarm(unsigned int seconds);

void _exit(int status);

__DECL_END

#endif //DUCKOS_UNISTD_H
