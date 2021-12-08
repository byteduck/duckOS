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

#include "../Widget.h"

namespace UI {
	class GridLayout: public Widget {
	public:
		WIDGET_DEF(GridLayout)

		void set_cells(const Dimensions& cell_size);
		Dimensions cells();

		//Widget
		void calculate_layout() override;
		Dimensions preferred_size() override;

	private:
		explicit GridLayout(const Dimensions& num_cells);

		Dimensions calculate_num_cells();
		Dimensions _num_cells;
	};
}

