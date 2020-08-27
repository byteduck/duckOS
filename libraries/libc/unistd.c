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
#include <libc/sys/syscall.h>
#include <assert.h>
#include <unistd.h>

char** environ = NULL;

pid_t fork() {
	return syscall(SYS_FORK);
}

int execv(const char* path, char* const argv[]) {
	return -1;
}

int execve(const char* filename, char* const argv[], char* const envp[]) {
	return syscall4(SYS_EXECVE, (int) filename, (int) argv, (int) envp);
}

int execvpe(const char* filename, char* const argv[], char* const envp[]) {
	return -1;
}

int execvp(const char* filename, char* const argv[]) {
	return syscall3(SYS_EXECVP, (int) filename, (int) argv);
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

ssize_t readlink(const char* path, char* buf, size_t size) {
	return syscall4(SYS_READLINK, (int) path, (int) buf, (int) size);
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

int dup(int fd) {
	return syscall2(SYS_DUP, fd);
}

int dup2(int old_fd, int new_fd) {
	return syscall3(SYS_DUP2, old_fd, new_fd);
}

int pipe(int pipefd[2]) {
	return syscall2(SYS_PIPE, (int) pipefd);
}

int pipe2(int pipefd[2], int flags) {
	return -1;
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
	return -1;
}

void _exit(int status) {
	syscall2(SYS_EXIT, status);
	assert(0);
	while(1);
}