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

#include "PageTable.h"
#include "PageDirectory.h"
#include "MemoryManager.h"

void PageTable::Entry::Data::set_address(size_t address) {
	page_addr = address >> 12u;
}

size_t PageTable::Entry::Data::get_address() {
	return page_addr << 12u;
}

PageTable::PageTable(size_t vaddr, bool alloc_table):
	_vaddr(vaddr)
{
	if(alloc_table) {
		m_entries_region = MM.alloc_contiguous_kernel_region(4096);
		_entries = (Entry*) m_entries_region->start();
	}
}

PageTable::PageTable(): _vaddr(0) {
}

PageTable::~PageTable() = default;

PageTable::Entry*& PageTable::entries() {
	return _entries;
}

PageTable::Entry &PageTable::operator[](int index) {
	return _entries[index];
}

size_t PageTable::vaddr() {
	return _vaddr;
}
