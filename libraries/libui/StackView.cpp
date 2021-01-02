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

#include "StackView.h"

UI::StackView::StackView(Direction direction): direction(direction) {

}

Dimensions UI::StackView::preferred_size() {
	return direction == HORIZONTAL ? Dimensions {current_pos, max_dim} : Dimensions {max_dim, current_pos};
}

void UI::StackView::on_child_added(UI::Widget* child) {
	if(direction == HORIZONTAL) {
		child->set_position(child->position() + Point {current_pos, 0});
		auto sz = child->preferred_size();
		current_pos += sz.width;
		if(sz.height > max_dim)
			max_dim = sz.height;
	} else if(direction == VERTICAL) {
		child->set_position(child->position() + Point {0, current_pos});
		auto sz = child->preferred_size();
		current_pos += sz.height;
		if(sz.width > max_dim)
			max_dim = sz.width;
	}
}
