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

#ifndef DUCKOS_PAGEDIRECTORY_H
#define DUCKOS_PAGEDIRECTORY_H

#include <common/cstddef.h>
#include "Memory.h"
#include "MemoryMap.h"
#include "LinkedMemoryRegion.h"
#include "PageTable.h"
#include <kernel/Result.hpp>

class PageTable;
class PageDirectory {
public:
	typedef union Entry {
		class __attribute((packed)) Data {
		public:
			bool present : 1;
			bool read_write : 1;
			bool user : 1;
			bool write_through : 1;
			bool cache_disable : 1;
			bool accessed : 1;
			bool zero : 1;
			uint8_t size : 1;
			bool ignored : 1;
			uint8_t unused : 3;
			size_t page_table_addr : 20;

			void set_address(size_t address);
			size_t get_address();
		} data;
		uint32_t value;
	} Entry;

	/**************************************
	 * Static kernel page directory stuff *
	 **************************************/

	static Entry kernel_entries[256];
	static MemoryMap kernel_vmem_map;
	static PageTable kernel_page_tables[256] __attribute__((aligned(4096)));
	static size_t kernel_page_tables_physaddr[1024];
	static MemoryRegion early_vmem_regions[3];
	static size_t used_kernel_pmem;
	static size_t used_kheap_pmem;

	/**
	 * Initialize the kernel page directory entries & related variables.
	 */
	static void init_kmem();

	/**
	 * Map the kernel into memory.
	 */
	static void map_kernel(MemoryRegion* text_region, MemoryRegion* data_region);

	/**
	 * Maps a number of pages at virtaddr (must be in kernel space) to physaddr
	 * @param region The linked memory region to map. The two regions must be equal in size and the virutal region must be above HIGHER_HALF.
	 * @param read_write Whether or not the page should be read/write.
	 */
	static void k_map_region(const LinkedMemoryRegion& region, bool read_write);

	/**
	 * Unmaps a memory region from kernel space.
	 * @param region The region to unmap. The regions are marked free.
	 */
	static void k_unmap_region(const LinkedMemoryRegion& region);

	/**
	 * Allocates a region of memory in kernel space and returns the region allocated.
	 * @param mem_size The amount of memory to allocate. Will be rounded up to be page-aligned.
	 * @return The LinkedMemoryRegion allocated.
	 */
	static LinkedMemoryRegion k_alloc_region(size_t mem_size);

	/**
	 * Allocates a region of memory for the heap. Should only be used by liballoc.
	 * @param mem_size The amount of memory to allocate. Will be rounded up to be page-aligned.
	 * @return A pointer to the region allocated.
	 */
	static void* k_alloc_region_for_heap(size_t mem_size);

	/**
	 * Called after liballoc allocates something. This is used to move the heap region buffer into alloced memory.
	 */
	static void k_after_alloc();

	/**
	 * Frees and unmaps a region of virtual and physical memory.
	 * @param region the region to be freed.
	 */
	static void k_free_region(const LinkedMemoryRegion& region);

	/**
	 * Frees and unmaps a region of virtual and physical memory.
	 * k_free_region(LinkedMemoryRegion) is faster, and is preferred when possible.
	 * @param virtaddr the virtual address of the region to be freed.
	 * @return Whether or not the region was freed.
	 */
	 static bool k_free_region(void* virtaddr);

	/**
	 * Maps a number of continguous pages in kernel vmem starting at physaddr.
	 * @param physaddr The physical address to map to.
	 * @param mem_size The amount of memory to map.
	 * @param read_write Whether or not the memory should be marked read/write.
	 * @return A pointer to where physaddr was mapped to.
	 */
	static void* k_mmap(size_t physaddr, size_t mem_size, bool read_write);

	/**
	 * Unmaps and frees number of contiguous pages in kernel vmem at virtaddr.
	 * @param virtaddr The virtual address within the region to be freed.
	 * @return Whether or not the region could be freed.
	 */
	static bool k_munmap(void* virtaddr);


	/************************************
	 * Per-process page directory stuff *
	 ************************************/

	PageDirectory();
	~PageDirectory();
	Entry* entries();

	/**
	 * Get the physical address of the table of entries.
	 * @return The physical address of the table of entries.
	 */
	size_t entries_physaddr();

	/**
	 * @return The entry at index.
	 */
	Entry& operator[](int index);

	/**
	 * Maps a number of pages at virtaddr (must be in program space) to physaddr.
	 * @param region The linked memory region to map. The two regions must be equal in size and the virutal region must be below HIGHER_HALF.
	 * @param read_write Whether or not the page should be read/write.
	 */
	void map_region(const LinkedMemoryRegion& region, bool read_write);

	/**
	 * Unmaps a memory region from program space.
	 * @param region The region to unmap. The regions are not marked free.
	 */
	void unmap_region(const LinkedMemoryRegion& region);

	/**
	 * Allocates a region of memory in program space and returns the region allocated.
	 * @param mem_size The amount of memory to allocate. Will be rounded up to be page-aligned.
	 * @return The LinkedMemoryRegion allocated.
	 */
	LinkedMemoryRegion allocate_region(size_t mem_size, bool read_write);

	/**
	 * Allocates a region of memory in program space starting at vaddr and returns the region allocated.
	 * @param vaddr The virtual address to start mapping at. Will be rounded down to be page-aligned.
	 * @param mem_size The amount of memory to allocate. Will be rounded up to be page-aligned.
	 * @return The LinkedMemoryRegion allocated.
	 */
	LinkedMemoryRegion allocate_region(size_t vaddr, size_t mem_size, bool read_write);

	/**
	 * Frees and unmaps a region of virtual and physical memory in program space.
	 * @param region the region to be freed.
	 */
	void free_region(const LinkedMemoryRegion& region);

	/**
	 * Frees an unmaps a region of virtual and physical memory in program space.
	 * free_region(LinkedMemoryRegion) is faster.
	 * @param virtaddr the virtual address of the region to be freed.
	 * @param virtaddr the size of the region to be freed.
	 * @return Whether or not the region was freed.
	 */
	bool free_region(size_t virtaddr, size_t size);

	/**
	 * Maps a number of continguous pages in program vmem starting at physaddr.
	 * @param physaddr The physical address to map to.
	 * @param mem_size The amount of memory to map.
	 * @param read_write Whether or not the memory should be marked read/write.
	 * @return A pointer to where physaddr was mapped to.
	 */
	void* mmap(size_t physaddr, size_t mem_size, bool read_write);

	/**
	 * Unmaps and frees number of contiguous pages in program vmem at virtaddr.
	 * @param virtaddr The virtual address within the region to be freed.
	 * @return Whether or not the region could be freed.
	 */
	bool munmap(void* virtaddr);

	/**
	 * Creates a region of memory in program space starting at vaddr (if nonzero) and returns the region allocated.
	 * @param vaddr The virtual address to start mapping at, or zero for unspecified. Will be rounded down to be page-aligned.
	 * @param mem_size The amount of memory to allocate. Will be rounded up to be page-aligned.
	 * @param pid The pid of the process that will own the region.
	 * @return The region created, or -ENOMEM if the region could not be created at the specified address.
	 */
	ResultRet<LinkedMemoryRegion> create_shared_region(size_t vaddr, size_t mem_size, pid_t pid);

	/**
	 * Attaches the shared memory region with the specified id to the address space.
	 * @param id The ID of the shared memory region to attach.
	 * @param vaddr If nonzero, the region will be attached to the specified address (rounded down to be page-aligned)
	 * @param pid The PID of the process attaching the region.
	 * @return The region attached, -ENOENT if it doesn't exist or if the process doesn't have permission to attach it, or -ENOMEM if it couldn't be attached.
	 */
	ResultRet<LinkedMemoryRegion> attach_shared_region(int id, size_t vaddr, pid_t pid);

	/**
	 * Detaches the shared memory region with the specified id from the address space.
	 * @param id The ID of the shared memory region to detach.
	 * @return 0 if successful, -ENOENT if it doesn't exist or wasn't attached.
	 */
	 Result detach_shared_region(int id);

	/**
	 * Allows a process access to a shared memory region.
	 * @param id The ID of the shared memory region to allow the process access to. Must be mapped in this page directory.
	 * @param called_pid The pid of the process requesting to add access. Will return -EPERM if not the owner of the region.
	 * @param pid The pid of the process to allow access to.
	 * @param write Whether to allow write access.
	 * @return 0 if successful, -ENOENT if it doesn't exist, -EPERM if not the owner of the region, -EEXIST if the process was already given certain permissions.
	 */
	Result allow_shared_region(int id, pid_t called_pid, pid_t pid, bool write);

	/**
	 * @return A pointer to the page directory's vmem map.
	 */
	MemoryMap& vmem_map();

	/**
	 * Gets the physical address for virtaddr.
	 * @param virtaddr The virtual address.
	 * @return The physical address for virtaddr.
	 */
	size_t get_physaddr(size_t virtaddr);

	/**
	 * Calls get_physaddr(size_t virtaddr).
	 */
	size_t get_physaddr(void* virtaddr);

	/**
	 * Allocates space for a new page table at tables_index in the page directory.
	 * @param tables_index The index in the page directory that this newly allocated page table is for.
	 * @return A pointer to the newly allocated page table.
	 */
	PageTable* alloc_page_table(size_t tables_index);

	/**
	 * Deallocates the space used for a page table at tables_index in the page directory.
	 * @param tables_index The index in the page directory of the page table being dealloc'd.
	 */
	void dealloc_page_table(size_t tables_index);

	/**
	 * Updates the entries for kernel space from the kernel page directory entries.
	 */
	void update_kernel_entries();

	/**
	 * Sets the entry pointer to entries. Should only be used for kernel page table.
	 */
	void set_entries(Entry* entries);

	/**
	 * Checks if a given virtual address is mapped to anything.
	 * @param vaddr The virtual address to check.
	 * @return Whether or not the given virtual address is mapped.
	 */
	bool is_mapped(size_t vaddr);

	/**
	 * Makes this page directory an identical copy of another, but with different physical memory.
	 * The page directory we're copying to (this) MUST be the loaded page directory.
	 * @param directory The page directory to fork from.
	 * @param parent_pid The PID of the parent process.
	 * @param new_pid The PID of the new process.
	 */
	void fork_from(PageDirectory *directory, pid_t parent_pid, pid_t new_pid);

	/**
	 * Tries to Copy-On-Write the page at virtaddr.
	 * @param virtaddr The virtual address to be CoWed.
	 * @return Whether or not the page was eligible for CoW and it was successful.
	 */
	bool try_cow(size_t virtaddr);

	/**
	 * Get the used memory in KiB
	 * @return The amount of used memory in KiB.
	 */
	size_t used_pmem();

	/**
	 * Get the used virtual memory in KiB
	 * @return The amount of used virtual memory in KiB.
	 */
	size_t used_vmem();

	/**
	 * Dump information about the memory maps
	 */
	void dump();

private:
	//The page directory entries for this page directory.
	Entry* _entries = nullptr;
	//The map of used vmem for this page directory.
	MemoryMap _vmem_map;
	//An array of pointers to the page tables that the directory points to.
	PageTable* _page_tables[768] = {nullptr};
	//An array of u16s that stores the number of pages mapped in each page table, used to deallocate a page table once no longer needed
	volatile int _page_tables_num_mapped[1024] = {0};
	//The used pmem in bytes.
	size_t _used_pmem = 0;
	//A lock used to prevent race conditions.
	SpinLock _lock;

};

#endif //DUCKOS_PAGEDIRECTORY_H
