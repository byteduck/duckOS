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

#include <sys/syscall.h>
#include <sys/mem.h>
#include <kernel/api/mmap.h>

void* memacquire(void* addr, size_t size) {
	struct mmap_args args = {
		addr,
		size,
		PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_ANONYMOUS
	};
	if(addr)
		args.flags |= MAP_FIXED;
	return (void*) syscall2(SYS_MMAP, (int) &args);
}

int memrelease(void* addr, size_t size) {
	return syscall3(SYS_MUNMAP, (int) addr, (size_t) size);
}

int shmcreate(void* addr, size_t size, struct shm* s) {
	return syscall4(SYS_SHMCREATE, (int) addr, (int) size, (int) s);
}

int shmattach(int id, void* addr, struct shm* s) {
	return syscall4(SYS_SHMATTACH, id, (int) addr, (int) s);
}

int shmdetach(int id) {
	return syscall2(SYS_SHMDETACH, id);
}

int shmallow(int id, pid_t pid, int perms) {
	return syscall4(SYS_SHMALLOW, id, pid, perms);
}