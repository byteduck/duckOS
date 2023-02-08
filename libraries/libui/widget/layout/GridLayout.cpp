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

GridLayout::GridLayout(const Gfx::Dimensions& num_cells): _num_cells(num_cells) {
	set_sizing_mode(FILL);
	set_uses_alpha(true);
}

void GridLayout::calculate_layout() {
	Gfx::Dimensions num_cells = calculate_num_cells();
	int col_size = current_size().width / num_cells.width;
	int col_remainder = current_size().width % num_cells.width;
	int row_size = current_size().height / num_cells.height;
	int row_remainder = current_size().height % num_cells.height;
	int col = 0;
	int x = 0, y = 0;
	for(auto child : children) {
		int width = col_size + (std::max(col_remainder--, 0) ? 1 : 0);
		int height = row_size + (std::max(row_remainder, 0) ? 1 : 0);
		child->set_layout_bounds({
			x,
			y,
			width,
			height
		});
		x += width;
		col++;
		if(col == num_cells.width) {
			col = 0;
			x = 0;
			y += height;
			row_remainder--;
			col_remainder = current_size().width % num_cells.width;
		}
	}
}

void GridLayout::set_cells(const Gfx::Dimensions& cells) {
	_num_cells = cells;
	update_layout();
}

Gfx::Dimensions GridLayout::cells() {
	return _num_cells;
}

Gfx::Dimensions GridLayout::preferred_size() {
	return calculate_total_dimensions([&](Duck::Ptr<Widget> widget) -> Gfx::Dimensions {
		return widget->preferred_size();
	});
}

Gfx::Dimensions GridLayout::minimum_size() {
	return calculate_total_dimensions([&](Duck::Ptr<Widget> widget) -> Gfx::Dimensions {
		return widget->minimum_size();
	});
}

Gfx::Dimensions GridLayout::calculate_num_cells() {
	if(_num_cells.width == 0 && _num_cells.height == 0) {
		if(!parent_window() && !parent())
			return {1, 1};
		Gfx::Dimensions parent_dims = parent_window() ? parent_window()->contents_rect().dimensions() : parent()->current_size();

		Gfx::Dimensions max_dims = {1, 1};
		for(auto& child : children) {
			auto sz = child->preferred_size();
			max_dims = {
				std::max(sz.width, max_dims.width),
				std::max(sz.height, max_dims.height)
			};
		}

		int width = std::max(1, parent_dims.width / max_dims.width);
		return {
			width,
			((int) children.size() + width - 1) / width
		};
	}

	Gfx::Dimensions actual_num_cells = _num_cells;
	if(_num_cells.width == 0)
		actual_num_cells.width = std::max(1, ((int) children.size() + _num_cells.height - 1) / std::max(1, _num_cells.height));
	if(_num_cells.height == 0)
		actual_num_cells.height = std::max(1, ((int) children.size() + _num_cells.width - 1) / std::max(1, _num_cells.width));
	return actual_num_cells;
}

Gfx::Dimensions GridLayout::calculate_total_dimensions(std::function<Gfx::Dimensions(Duck::Ptr<Widget>)> dim_func) {
	Gfx::Dimensions max_dims = {1, 1};
	for(auto& child : children) {
		auto sz = dim_func(child);
		if(sz.width > max_dims.width)
			max_dims.width = sz.width;
		if(sz.height > max_dims.height)
			max_dims.height = sz.height;
	}

	Gfx::Dimensions actual_num_cells = calculate_num_cells();
	return {max_dims.width * actual_num_cells.width, max_dims.height * actual_num_cells.height};
}
