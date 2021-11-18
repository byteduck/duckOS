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

#include "ListScrollable.h"

using namespace UI;

void ListScrollable::calculate_layout() {
}

void ListScrollable::on_layout_change(const Rect& old_rect) {
	do_update(true);
}

void ListScrollable::on_scroll(Point scroll) {
	do_update(false);
}

Dimensions ListScrollable::scrollable_area() {
	//If the item dimensions haven't been calculated, calculate them
	if(_item_dims.width == -1)
		_item_dims = preferred_item_dimensions();
	return {_item_dims.width, _item_dims.height * num_items()};
}

void ListScrollable::update_item(int index) {
	//If we haven't rendered the list for the first time yet, just return
	if(_item_dims.width == -1)
		return;

	//If the item isn't visible, just return
	if(index < _prev_first_visible || index > _prev_last_visible)
		return;

	//If it is, update it
	if(_items[index]) {
		remove_child(_items[index]);
		_items.erase(index);
	}

	_items[index] = setup_entry(index);
}

void ListScrollable::update_data() {
	do_update(false);
}

void ListScrollable::do_update(bool dimensions_changed) {
	_item_dims = {current_size().width, preferred_item_dimensions().height};

	int first = scroll_position().y / _item_dims.height;
	int last = (scroll_position().y + current_size().height) / _item_dims.height;
	int num = num_items();
	if(first < 0)
		first = 0;
	if(last >= num)
		last = num - 1;

	//Remove old, unused items
	for(int i = _prev_first_visible; i < first; i++) {
		if(_items[i]) {
			remove_child(_items[i]);
			_items.erase(i);
		}
	}
	for(int i = _prev_last_visible; i > last; i--) {
		if(_items[i]) {
			remove_child(_items[i]);
			_items.erase(i);
		}
	}

	_prev_first_visible = first;
	_prev_last_visible = last;

	//Create new items and move items around
	for(int i = first; i <= last; i++) {
		auto pos = Point {0, i * _item_dims.height} - scroll_position();
		if(!_items[i])
			_items[i] = setup_entry(i);
		else if(dimensions_changed)
			_items[i]->set_layout_bounds({pos, _item_dims});
		else
			_items[i]->set_position_nolayout(pos);
	}
}

Widget::Ptr ListScrollable::setup_entry(int index) {
	auto widget = create_entry(index);
	add_child(widget);
	widget->set_layout_bounds({
		Point {0, index * _item_dims.height} - scroll_position(),
		_item_dims
	});
	return widget;
}

ListScrollable::ListScrollable() {
	set_uses_alpha(true);
}
