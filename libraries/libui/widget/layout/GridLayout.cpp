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

#include "GridLayout.h"
#include "../../Window.h"

using namespace UI;

GridLayout::GridLayout(const Dimensions& num_cells): _num_cells(num_cells) {
	set_sizing_mode(FILL);
	set_uses_alpha(true);
}

void GridLayout::calculate_layout() {
	Dimensions num_cells = calculate_num_cells();
	int col_size = current_size().width / num_cells.width;
	int row_size = current_size().height / num_cells.height;
	int col = 0, row = 0;
	for(auto child : children) {
		child->set_layout_bounds({
			col * col_size,
			row * row_size,
			col_size,
			row_size
		});
		col++;
		if(col == num_cells.width) {
			col = 0;
			row++;
		}
	}
}

void GridLayout::set_cells(const Dimensions& cells) {
	_num_cells = cells;
	update_layout();
}

Dimensions GridLayout::cells() {
	return _num_cells;
}

Dimensions GridLayout::preferred_size() {
	Dimensions max_dims = {1, 1};
	for(auto& child : children) {
		auto sz = child->preferred_size();
		if(sz.width > max_dims.width)
			max_dims.width = sz.width;
		if(sz.height > max_dims.height)
			max_dims.height = sz.height;
	}

	Dimensions actual_num_cells = calculate_num_cells();
	return {max_dims.width * actual_num_cells.width, max_dims.height * actual_num_cells.height};
}

Dimensions GridLayout::calculate_num_cells() {
	if(_num_cells.width == 0 && _num_cells.height == 0) {
		if(!parent_window() && !parent())
			return {1, 1};
		Dimensions parent_dims = parent_window() ? parent_window()->contents_rect().dimensions() : parent()->current_size();

		Dimensions max_dims = {1, 1};
		for(auto& child : children) {
			auto sz = child->preferred_size();
			if(sz.width > max_dims.width)
				max_dims.width = sz.width;
			if(sz.height > max_dims.height)
				max_dims.height = sz.height;
		}

		int width = parent_dims.width / max_dims.width;
		if(width == 0)
			width = 1;
		return {
			width,
			((int) children.size() + width - 1) / width
		};
	}

	Dimensions actual_num_cells = _num_cells;
	if(_num_cells.width == 0)
		actual_num_cells.width = ((int) children.size() + _num_cells.height - 1) / _num_cells.height;
	if(_num_cells.height == 0)
		actual_num_cells.height = ((int) children.size() + _num_cells.width - 1) / _num_cells.width;
	return actual_num_cells;
}
