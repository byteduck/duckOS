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

#include "BoxLayout.h"
#include "libui/Theme.h"

UI::BoxLayout::BoxLayout(Direction direction, int spacing): direction(direction), spacing(spacing) {
	set_uses_alpha(true);
	set_sizing_mode(UI::FILL);
}

Dimensions UI::BoxLayout::preferred_size() {
	int max_dim = 0;
	int current_pos = 0;
	for(auto& child : children) {
		auto sz = child->preferred_size();
		if(direction == HORIZONTAL) {
			current_pos += sz.width + spacing;
			if(sz.height > max_dim)
				max_dim = sz.height;
		} else if(direction == VERTICAL) {
			current_pos += sz.height + spacing;
			if(sz.width > max_dim)
				max_dim = sz.width;
		}
	}
	if(current_pos >= spacing)
		current_pos -= spacing;
	return direction == HORIZONTAL ? Dimensions {current_pos, max_dim} : Dimensions {max_dim, current_pos};
}

void UI::BoxLayout::set_spacing(int new_spacing) {
	spacing = new_spacing;
	update_layout();
}

void UI::BoxLayout::calculate_layout() {
	int pos = 0;
	int i = 0;
	Dimensions size = current_size();
	for(auto child : children) {
		auto preferred_size = child->preferred_size();

		//If the last child's sizing mode is fill, then resize it to fit the rest of the space
		if(i == children.size() - 1 && child->sizing_mode() == FILL) {
			if(direction == VERTICAL)
				child->set_layout_bounds({0, pos, size.width, size.height - pos});
			else
				child->set_layout_bounds({pos, 0, size.width - pos, size.height});
		} else {
			if(direction == VERTICAL)
				child->set_layout_bounds({0, pos, size.width, preferred_size.height});
			else
				child->set_layout_bounds({pos, 0, preferred_size.width, size.height});
		}

		if(direction == VERTICAL)
			pos += preferred_size.height;
		else
			pos += preferred_size.width;
		pos += spacing;

		i++;
	}
}