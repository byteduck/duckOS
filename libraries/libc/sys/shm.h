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

#pragma once

#include <sys/types.h>
#include <sys/cdefs.h>
#include <kernel/api/shm.h>
#include <kernel/api/page_size.h>

__DECL_BEGIN

/**
 * Allocates an area of memory that can be shared. Child processes created with fork() will automatically have access.
 * @param addr NULL or a specific address that the memory should be allocated at.
 * @param size he minimum amount of memory to be allocated (after the start address, if specified)
 * @param s A pointer to a struct shm where information about the allocated memory will be stored.
 * @param name A name to described the shared region.
 * @return 0 if successful, -1 if not.
 */
int shmcreate_named(void* addr, size_t size, struct shm* s, const char* name);

/**
 * Allocates an area of memory that can be shared. Child processes created with fork() will automatically have access.
 * @param addr NULL or a specific address that the memory should be allocated at.
 * @param size he minimum amount of memory to be allocated (after the start address, if specified)
 * @param s A pointer to a struct shm where information about the allocated memory will be stored.
 * @return 0 if successful, -1 if not.
 */
int shmcreate(void* addr, size_t size, struct shm* s);

/**
 * Attaches an already-created area of shared memory to the program.
 * @param id The ID, given in struct shm by shmcreate(), of the shared memory area to attach.
 * @param addr NULL, or a specific address to attach the memory to. Will be rounded down to be page-aligned.
 * @param s A pointer to a struct shm where information about the attached memory area will be stored.
 * @return 0 if successful, -1 if not.
 */
int shmattach(int id, void* addr, struct shm* s);

/**
 * Detaches an area of shared memory from the program. If this program is the last one with access to the memory, it will be destroyed.
 * @param id The ID of the shared memory area to detach from.
 * @return 0 if successful, -1 if not.
 */
int shmdetach(int id);

/**
 * Allows another process access to a shared memory segment with certain permissions. CANNOT be used to revoke permissions.
 * @param id The ID of the shared memory segment to allow access to.
 * @param pid The pid of the process to allow.
 * @param perms The permissions to allow. (SHM_READ, SHM_WRITE, and SHM_SHARE)
 * @return
 */
int shmallow(int id, pid_t pid, int perms);

__DECL_END
