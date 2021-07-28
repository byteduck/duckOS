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

#include "PageTable.h"
#include "PageDirectory.h"
#include "LinkedMemoryRegion.h"

void PageTable::Entry::Data::set_address(size_t address) {
	page_addr = address >> 12u;
}

size_t PageTable::Entry::Data::get_address() {
	return page_addr << 12u;
}

PageTable::PageTable(size_t vaddr, PageDirectory* page_directory, bool alloc_table):
_vaddr(vaddr), _page_directory(page_directory), _alloced_table(alloc_table) {
	if(alloc_table) _entries = (Entry*) PageDirectory::k_alloc_region(4096).virt->start;
}

PageTable::PageTable(): _vaddr(0), _page_directory(nullptr), _alloced_table(false) {
}

PageTable::~PageTable() {
	if(_alloced_table) PageDirectory::k_free_region(_entries);
}

PageTable::Entry*& PageTable::entries() {
	return _entries;
}

PageTable::Entry &PageTable::operator[](int index) {
	return _entries[index];
}

size_t PageTable::vaddr() {
	return _vaddr;
}

namespace Memory {

}
