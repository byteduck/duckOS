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
#include <stddef.h>
#include <sys/syscall.h>
#include <assert.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char** environ = NULL;
char** __original_environ = NULL;

pid_t fork() {
	return syscall(SYS_FORK);
}

pid_t vfork() {
	return fork();
}

int execv(const char* path, char* const argv[]) {
	return execve(path, argv, environ);
}

int execve(const char* filename, char* const argv[], char* const envp[]) {
	return syscall4(SYS_EXECVE, (int) filename, (int) argv, (int) envp);
}

int execvpe(const char* filename, char* const argv[], char* const envp[]) {
	char cwd[512];
	getcwd(cwd, 512);

	// If the path contains a slash, ignore the path
	if(strchr(filename, '/'))
		execve(filename, argv, envp);

	// Get the path
	char* path = getenv("PATH");
	if(!path)
		path = DEFAULT_PATH;
	path = strdup(path);

	// Try each element of path
	char* path_elem = strtok(path, ":");
	while(path_elem) {
		// Create a string with the full path of the executable
		size_t path_len = strlen(path_elem);
		char* full_path = malloc(path_len + strlen(filename) + 2);
		strcpy(full_path, path_elem);
		full_path[path_len] = '/';
		strcpy(full_path + path_len + 1, filename);

		// Try executing it
		int res = execve(full_path, argv, envp);
		free(full_path);
		if(res && errno != ENOENT)
			return res;
		path_elem = strtok(NULL, ":");
	}

	free(path);
	errno = ENOENT;
	return -1;
}

int execvp(const char* filename, char* const argv[]) {
	return execvpe(filename, argv, environ);
}

int execl(const char* filename, const char* arg, ...) {
	return -1;
}

int execlp(const char* filename, const char* arg, ...) {
	return -1;
}

pid_t getpid() {
	return syscall(SYS_GETPID);
}

pid_t getppid() {
	return syscall(SYS_GETPPID);
}

pid_t getsid(pid_t pid) {
	return syscall(SYS_GETSID);
}

pid_t setsid() {
	return syscall(SYS_SETSID);
}

int setpgid(pid_t pid, pid_t pgid) {
	return syscall3(SYS_SETPGID, pid, pgid);
}

pid_t getpgid(pid_t pid) {
	return syscall2(SYS_GETPGID, pid);
}

pid_t getpgrp() {
	return syscall(SYS_GETPGRP);
}

uid_t getuid() {
	return syscall(SYS_GETUID);
}

uid_t geteuid() {
	return syscall(SYS_GETEUID);
}

gid_t getgid() {
	return syscall(SYS_GETGID);
}

gid_t getegid() {
	return syscall(SYS_GETEGID);
}

int getgroups(int ngroups, gid_t list[]) {
	return syscall3(SYS_GETGROUPS, ngroups, (int) list);
}

int setgroups(size_t ngroups, const gid_t* list) {
	return syscall3(SYS_SETGROUPS, ngroups, (int) list);
}

int setuid(uid_t uid) {
	return syscall2(SYS_SETUID, uid);
}

int setgid(gid_t gid) {
	return syscall2(SYS_SETGID, gid);
}

ssize_t read(int fd, void* buf, size_t count) {
	return syscall4(SYS_READ, fd, (int) buf, count);
}

ssize_t pread(int fd, void* buf, size_t count, off_t offset) {
	return -1;
}

ssize_t write(int fd, const void* buf, size_t count) {
	return syscall4(SYS_WRITE, fd, (int) buf, count);
}

ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset) {
	return -1;
}

off_t lseek(int fd, off_t off, int whence) {
	return syscall4(SYS_LSEEK, fd, off, whence);
}

int fchown(int fd, uid_t uid, gid_t gid) {
	return syscall4(SYS_FCHOWN, fd, uid, gid);
}

int ftruncate(int fd, off_t length) {
	return syscall3(SYS_FTRUNCATE, fd, length);
}

int close(int fd) {
	return syscall2(SYS_CLOSE, fd);
}

int isatty(int fd) {
	return syscall2(SYS_ISATTY, fd) == 1 ? 1 : 0;
}

ssize_t readlink(const char* path, char* buf, size_t size) {
	return syscall4(SYS_READLINK, (int) path, (int) buf, (int) size);
}

ssize_t readlinkat(int fd, const char* path, char* buf, size_t size) {
	struct readlinkat_args args = {fd, path, buf, size};
	return syscall2(SYS_READLINKAT, (int) &args);
}

int link(const char* oldpath, const char* newpath) {
	return syscall3(SYS_LINK, (int) oldpath, (int) newpath);
}

int unlink(const char* pathname) {
	return syscall2(SYS_UNLINK, (int) pathname);
}

int symlink(const char* target, const char* linkname) {
	return syscall3(SYS_SYMLINK, (int) target, (int) linkname);
}

int rmdir(const char* pathname) {
	return syscall2(SYS_RMDIR, (int) pathname);
}

int chown(const char* pathname, uid_t uid, gid_t gid) {
	return syscall4(SYS_CHOWN, (int) pathname, (int) uid, (int) gid);
}

int truncate(const char* path, off_t length) {
	return syscall3(SYS_TRUNCATE, (int) path, (int) length);
}

int access(const char* path, int how) {
	return syscall3(SYS_ACCESS, (int) path, (int) how);
}

int dup(int fd) {
	return syscall2(SYS_DUP, fd);
}

int dup2(int old_fd, int new_fd) {
	return syscall3(SYS_DUP2, old_fd, new_fd);
}

int pipe(int pipefd[2]) {
	return syscall3(SYS_PIPE, (int) pipefd, 0);
}

int pipe2(int pipefd[2], int flags) {
	return syscall3(SYS_PIPE, (int) pipefd, flags);
}

long pathconf(const char* path, int name) {
	return fpathconf(0, name);
}

long fpathconf(int fd, int name) {
	switch (name) {
	case _PC_NAME_MAX:
		return NAME_MAX;
	case _PC_PATH_MAX:
		return PATH_MAX;
	case _PC_VDISABLE:
		return _POSIX_VDISABLE;
	case _PC_LINK_MAX:
		return LINK_MAX;
	default:
		return 0;
	}
}

int chdir(const char* pathname) {
	return syscall2(SYS_CHDIR, (int) pathname);
}

int fchdir(int fd) {
	return -1;
}

char* getcwd(char* buf, size_t size) {
	return (char*) syscall3(SYS_GETCWD, (int) buf, (int) size);
}

char* getwd(char* buf) {
	return (char*) syscall2(SYS_GETCWD, (int) buf);
}

int sleep(unsigned secs) {
	struct timespec time = {secs, 0};
	struct timespec remainder;
	if(syscall3_noerr(SYS_SLEEP, (int) &time, (int) &remainder) < 0)
		return remainder.tv_sec;
	return 0;
}

int usleep(useconds_t usec) {
	struct timespec time = {0, usec * 1000};
	struct timespec remainder;
	return syscall3(SYS_SLEEP, (int) &time, (int) &remainder);
}

pid_t tcgetpgrp(int fd) {
	return ioctl(fd, TIOCGPGRP);
}

int tcsetpgrp(int fd, pid_t pgid) {
	return ioctl(fd, TIOCSPGRP, pgid);
}

unsigned int alarm(unsigned int seconds) {
	return -1; // TODO
}

void _exit(int status) {
	syscall2(SYS_EXIT, status);
	__builtin_unreachable();
}