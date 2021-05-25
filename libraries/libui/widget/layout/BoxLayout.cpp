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

Rect UI::BoxLayout::bounds_for_child(UI::Widget *child) {
    int pos = 0;
    Dimensions size = current_size();
    for(auto it_child : children) {
        if(it_child == child) {
            //If we've gotten to the child in question, return its calculated bounds
            if(direction == VERTICAL)
                return {0, pos, size.width, child->preferred_size().height};
            else
                return {pos, 0, child->preferred_size().width, size.height};
        } else {
            //Otherwise, keep calculating
            Dimensions child_dims = it_child->preferred_size();
            if(direction == VERTICAL)
                pos += child_dims.height;
            else
                pos += child_dims.width;
            pos += spacing;
        }
    }
    return {0, 0, 0, 0};
}