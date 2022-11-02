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

void UI::FlexLayout::set_spacing(int spacing) {
	m_spacing = spacing;
	update_layout();
}

Gfx::Dimensions UI::FlexLayout::preferred_size() {
	int main_dim = 0;
	int max_other_dim = 0;
	for(auto& child : children) {
		auto sz = child->preferred_size();
		if(direction == HORIZONTAL) {
			main_dim += sz.width;
			max_other_dim = std::max(max_other_dim, sz.height);
		} else if(direction == VERTICAL) {
			main_dim += sz.height;
			max_other_dim = std::max(max_other_dim, sz.width);
		}
	}

	if(!children.empty())
		main_dim += (children.size() - 1) * m_spacing;

	if(direction == HORIZONTAL)
		return Gfx::Dimensions {main_dim, max_other_dim};
	else
		return Gfx::Dimensions {max_other_dim, main_dim};
}

void UI::FlexLayout::calculate_layout() {
	Gfx::Dimensions size = current_size();

	int nonfill_size = 0;
	int num_fill = 0;
	int total_spacing = (children.size() - 1) * m_spacing;;

	for(int i = 0; i < children.size(); i++) {
		if(children[i]->sizing_mode() != FILL) {
			if(direction == VERTICAL)
				nonfill_size += children[i]->preferred_size().height;
			else
				nonfill_size += children[i]->preferred_size().width;
		} else {
			num_fill++;
		}
	}

	int fill_size = 1;
	if(num_fill && nonfill_size + total_spacing < (direction == VERTICAL ? size.height : size.width))
		fill_size = ((direction == VERTICAL ? size.height : size.width) - nonfill_size - total_spacing) / num_fill;

	int cur_pos = 0;
	for(int i = 0; i < children.size(); i++) {
		if(direction == VERTICAL) {
			int cell_size = children[i]->sizing_mode() == FILL ? fill_size : children[i]->preferred_size().height;
			children[i]->set_layout_bounds({0, cur_pos, size.width, cell_size});
			cur_pos += cell_size + m_spacing;
		} else {
			int cell_size = children[i]->sizing_mode() == FILL ? fill_size : children[i]->preferred_size().width;
			children[i]->set_layout_bounds({cur_pos, 0, cell_size, size.height});
			cur_pos += cell_size + m_spacing;
		}
	}
}
