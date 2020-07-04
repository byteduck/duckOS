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

#ifndef DUCKOS_COWPARENT_H
#define DUCKOS_COWPARENT_H

#include "PageTable.h"
#include <common/shared_ptr.hpp>

namespace Paging {
	class PageTable;
	class CoWParent {
	public:
		CoWParent(const DC::shared_ptr<PageTable>& table, int entry);
		~CoWParent();

		DC::shared_ptr<PageTable> table();
		int table_entry();
	private:
		DC::shared_ptr<PageTable> _table;
		int _table_entry;
	};
}


#endif //DUCKOS_COWPARENT_H
