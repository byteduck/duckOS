#include "PageTable.h"

namespace Paging {
	void PageTable::Entry::Data::set_address(size_t address) {
		page_addr = address >> 12u;
	}

	size_t PageTable::Entry::Data::get_address() {
		return page_addr << 12u;
	}

	PageTable::PageTable() = default;

	PageTable::Entry *PageTable::entries() {
		return _entries;
	}

	PageTable::Entry &PageTable::operator[](int index) {
		return _entries[index];
	}
}
