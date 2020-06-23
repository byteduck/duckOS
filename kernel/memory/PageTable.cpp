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

namespace Paging {
	void PageTable::Entry::Data::set_address(size_t address) {
		page_addr = address >> 12u;
	}

	size_t PageTable::Entry::Data::get_address() {
		return page_addr << 12u;
	}

	PageTable::PageTable(Entry* entries_ptr) {
		_entries = entries_ptr;
	}

	PageTable::~PageTable() {
	}

	PageTable::Entry *PageTable::entries() {
		return _entries;
	}

	PageTable::Entry &PageTable::operator[](int index) {
		return _entries[index];
	}

	void PageTable::set_copy_on_write() {
		if(_entries) {
			for(auto i = 0; i < 1024; i++) _entries[i].data.read_write = false;
		}
		_copy_on_write = true;
	}

	bool PageTable::is_copy_on_write() {
		return _copy_on_write;
	}

}
