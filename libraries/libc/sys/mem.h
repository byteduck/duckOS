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

#ifndef DUCKOS_LIBC_MMAP_H
#define DUCKOS_LIBC_MMAP_H

#include <sys/types.h>
#include <sys/cdefs.h>

#define PAGE_SIZE 4096

#define SHM_READ 0x1u
#define SHM_WRITE 0x2u

__DECL_BEGIN

/**
 * Request memory to be allocated to the program.
 * @param addr NULL or a specific address that the memory should be allocated at.
 * @param size The minimum amount of memory to be allocated (after the start address, if specified)
 * @return The page-aligned address of the memory allocated. If an address was specified, this may be rounded down to be page-aligned.
 */
void* memacquire(void* addr, size_t size);

/**
 * Release memory from the program.
 * @param addr The address containing the memory to release. Will be rounded down to be page-aligned.
 * @param size The amount of memory to free. Will be rounded up to be page-aligned.
 * @return 0 if successful, -1 if not.
 */
int memrelease(void* addr, size_t size);

struct shm {
	void* ptr;
	size_t size;
	int id;
} __attribute__((aligned(16)));

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
 * @param perms The permissions to allow. (SHM_READ and SHM_WRITE)
 * @return
 */
int shmallow(int id, pid_t pid, int perms);

__DECL_END

#endif //DUCKOS_LIBC_MMAP_H
