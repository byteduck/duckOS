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

#include <kernel/tasking/TaskManager.h>
#include "PageTable.h"
#include "PageDirectory.h"

namespace Paging {
	void PageTable::Entry::Data::set_address(size_t address) {
		page_addr = address >> 12u;
	}

	size_t PageTable::Entry::Data::get_address() {
		return page_addr << 12u;
	}

	PageTable::PageTable(size_t vaddr, PageDirectory* page_directory, bool alloc_table):
	_vaddr(vaddr), _page_directory(page_directory), _alloced_table(alloc_table) {
		if(alloc_table) _entries = (Entry*) PageDirectory::k_alloc_pages(4096);
	}

	PageTable::PageTable(): _vaddr(0), _page_directory(nullptr), _alloced_table(false) {
	}

	PageTable::~PageTable() {
		if(_is_cow_parent) {
			for(auto i = 0; i < 1024; i++) {
				if(!cow_parents[i] && _entries[i].data.present)
					pmem_bitmap().set_page_free(_entries[i].data.get_address() / PAGE_SIZE);
			}
		}
		for(auto & cow_parent : cow_parents) {
			cow_parent.reset();
		}
		if(_alloced_table) PageDirectory::k_free_pages(_entries, 4096);
	}

	PageTable::Entry*& PageTable::entries() {
		return _entries;
	}

	PageTable::Entry &PageTable::operator[](int index) {
		return _entries[index];
	}

	bool PageTable::cow(size_t vaddr) {
		if(vaddr < _vaddr || vaddr >= _vaddr + PAGE_SIZE * 1024)
		if(!_entries || vaddr < _vaddr || vaddr >= _vaddr + PAGE_SIZE * 1024) return false;

		auto entry = (vaddr - _vaddr) / PAGE_SIZE;
		if(!cow_parents[entry]) return false;

		size_t page_vaddr = (vaddr / PAGE_SIZE) * PAGE_SIZE;

		//Allocate new page in kernel space and copy page into it
		void* tmpbuf = PageDirectory::k_alloc_pages(PAGE_SIZE);
		memcpy(tmpbuf, (void*)page_vaddr, PAGE_SIZE);

		//Remap page to new page
		_page_directory->unmap_page(page_vaddr);
		_page_directory->map_page(_page_directory->get_physaddr(tmpbuf), page_vaddr, true);
		_page_directory->take_pmem_ownership(_page_directory->get_physaddr(tmpbuf));

		//Unmap from kernel
		PageDirectory::k_unmap_page((size_t)tmpbuf);

		cow_parents[entry].reset();

		return true;
	}

	void PageTable::setup_cow_entry(const DC::shared_ptr<CoWParent>& parent) {
		_entries[parent->table_entry()].value = parent->table()->_entries[parent->table_entry()].value;
		_entries[parent->table_entry()].data.read_write = false;
		cow_parents[parent->table_entry()] = parent;
	}

	size_t PageTable::vaddr() {
		return _vaddr;
	}

	void PageTable::mark_cow_parent() {
		_is_cow_parent = true;
	}

	DC::shared_ptr<CoWParent> PageTable::get_cow_parent(int i) {
		ASSERT(i < 1024 && i > 0);
		return cow_parents[i];
	}

	void PageTable::reset_cow_parent(int i) {
		ASSERT(i < 1024 && i > 0);
		ASSERT(cow_parents[i]);
		cow_parents[i].reset();
		_entries[i].value = 0;
	}
}
