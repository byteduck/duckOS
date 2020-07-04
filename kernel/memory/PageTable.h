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

#ifndef DUCKOS_PAGETABLE_H
#define DUCKOS_PAGETABLE_H

#include <common/cstddef.h>
#include <common/shared_ptr.hpp>
#include "MemoryBitmap.hpp"
#include "PageDirectory.h"
#include "paging.h"
#include "CoWParent.h"

namespace Paging {
	class PageDirectory;
	class CoWParent;
	class PageTable {
	public:
		typedef union Entry {
			struct __attribute((packed)) Data {
			public:
				bool present : 1;
				bool read_write : 1;
				bool user : 1;
				bool write_through : 1;
				bool cache_disabled : 1;
				bool acessed: 1;
				bool dirty : 1;
				bool zero : 1;
				bool global : 1;
				uint8_t unused : 3;
				uint32_t page_addr : 20;

				void set_address(size_t address);
				size_t get_address();
			} data;
			uint32_t value;
		} Entry;

		PageTable(size_t vaddr, PageDirectory* page_directory, bool alloc_table = true);
		PageTable();
		~PageTable();

		/**
		 * Called to copy-on-write when a write fault happens in this table.
		 * If the entry in question is set in cow_pages, CoW will be performed and true will be returned.
		 * Assumes that this table is loaded.
		 * @return Whether or not CoW was performed.
		 */
		bool cow(size_t vaddr);

		/**
		 * Sets up CoW for an entry.
		 * @param parent The CoWParent object.
		 */
		void setup_cow_entry(const DC::shared_ptr<CoWParent>& parent);

		/**
		 * Returns the vaddr of the beginning of the block of memory corresponding to this page table.
		 * @return The table's virtual address.
		 */
		size_t vaddr();

		/**
		 * Marks this PageTable as a CoW parent so that its physical memory will be freed on destruction.
		 */
		void mark_cow_parent();

		DC::shared_ptr<CoWParent> get_cow_parent(int index);
		void reset_cow_parent(int i);

		Entry*& entries();
		Entry& operator[](int index);

	private:
		Entry *_entries = nullptr;
		size_t _vaddr = 0;
		PageDirectory* _page_directory = nullptr;
		DC::shared_ptr<CoWParent> cow_parents[1024];
		bool _alloced_table = false;
		bool _is_cow_parent = false;
	};
}

#endif //DUCKOS_PAGETABLE_H
