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

#include "ContainerView.h"

using namespace UI;

ContainerView::ContainerView() {
	set_uses_alpha(true);
}

void ContainerView::on_scroll(Gfx::Point new_position) {
	if(_contents)
		_contents->set_position_nolayout(new_position * -1);
}

Gfx::Dimensions ContainerView::scrollable_area() {
	return _contents->preferred_size();
}

void ContainerView::set_contents(Duck::Ptr<Widget> contents) {
	if(_contents)
		return;
	_contents = std::move(contents);
	add_child(_contents);
	update_layout();
}

void ContainerView::set_show_scrollbar(bool show_scrollbar) {
	ScrollView::set_show_scrollbar(show_scrollbar);
	calculate_layout();
}

void ContainerView::calculate_layout() {
	if(_contents) {
		auto usable_area = content_area();
		auto size = _contents->preferred_size();
		if(_contents->sizing_mode() == FILL)
			size.width = usable_area.width;
		_contents->set_layout_bounds({
			-scroll_position().x + usable_area.x,
			-scroll_position().y + usable_area.y,
			size.width,
			size.height
		});
	}
}

Gfx::Dimensions ContainerView::preferred_size() {
	return _contents->preferred_size();
}