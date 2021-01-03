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

UI::StackView::StackView(Direction direction, int spacing): direction(direction), spacing(spacing) {

}

Dimensions UI::StackView::preferred_size() {
	max_dim = 0;
	current_pos = 0;
	for(auto& child : children) {
		if(direction == HORIZONTAL) {
			child->set_position(Point{current_pos, 0});
			auto sz = child->current_size();
			current_pos += sz.width + spacing;
			if(sz.height > max_dim)
				max_dim = sz.height;
		} else if(direction == VERTICAL) {
			child->set_position(Point{0, current_pos});
			auto sz = child->current_size();
			current_pos += sz.height + spacing;
			if(sz.width > max_dim)
				max_dim = sz.width;
		}
	}
	return direction == HORIZONTAL ? Dimensions {current_pos, max_dim} : Dimensions {max_dim, current_pos};
}

void UI::StackView::set_spacing(int new_spacing) {
	spacing = new_spacing;
	update_size();
}

void UI::StackView::on_child_added(UI::Widget* child) {
	update_size();
}

void UI::StackView::do_repaint(Image& fb) {
	fb.fill({0, 0, fb.width, fb.height}, RGB(40, 40, 40));
}
