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

#include "FlexLayout.h"

UI::FlexLayout::FlexLayout(UI::FlexLayout::Direction direction): direction(direction) {
	set_uses_alpha(true);
	set_sizing_mode(UI::FILL);
}

Gfx::Dimensions UI::FlexLayout::preferred_size() {
	int max_main_dim = 0;
	int max_other_dim = 0;
	for(auto& child : children) {
		auto sz = child->preferred_size();
		if(direction == HORIZONTAL) {
			if(sz.width > max_main_dim)
				max_main_dim = sz.width;
			if(sz.height > max_other_dim)
				max_other_dim = sz.height;
		} else if(direction == VERTICAL) {
			if(sz.height > max_main_dim)
				max_main_dim = sz.height;
			if(sz.width > max_other_dim)
				max_other_dim = sz.width;
		}
	}

	int size = max_main_dim * (int) children.size();

	if(direction == HORIZONTAL)
		return Gfx::Dimensions {size, max_other_dim};
	else
		return Gfx::Dimensions {max_other_dim, size};
}

void UI::FlexLayout::calculate_layout() {
	Gfx::Dimensions size = current_size();
	for(int i = 0; i < children.size(); i++) {
		if(direction == VERTICAL) {
			int cell_size = size.height / (int) children.size();
			int cell_position = cell_size * i;
			children[i]->set_layout_bounds({0, cell_position, size.width, cell_size});
		} else {
			int cell_size = size.width / (int) children.size();
			int cell_position = cell_size * i;
			children[i]->set_layout_bounds({cell_position, 0, cell_size, size.height});
		}
	}
}
