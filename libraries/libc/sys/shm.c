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
#include <sys/shm.h>
#include <kernel/api/mmap.h>

int shmcreate(void* addr, size_t size, struct shm* s) {
	return shmcreate_named(addr, size, s, 0);
}

int shmcreate_named(void* addr, size_t size, struct shm* s, const char* name) {
	struct shmcreate_args args;
	args.addr = addr;
	args.size = size;
	args.shm = s;
	args.name = name;
	return syscall2(SYS_SHMCREATE, (int) &args);
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