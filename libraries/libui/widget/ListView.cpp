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

#include "ListView.h"

using namespace UI;

void ListView::calculate_layout() {
	do_update(true);
}

void ListView::on_scroll(Gfx::Point scroll) {
	do_update(false);
}

Gfx::Dimensions ListView::scrollable_area() {
	if(delegate.expired())
		return {1, 1};
	auto locked_delegate = this->delegate.lock();

	//If the item dimensions haven't been calculated, calculate them
	if(_item_dims.width == -1)
		_item_dims = locked_delegate->lv_preferred_item_dimensions();
	return {_item_dims.width, _item_dims.height * ((locked_delegate->lv_num_items() + _num_per_row - 1) / _num_per_row)};
}

void ListView::update_item(int index) {
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

void ListView::update_data() {
	for(auto& item_pair : _items) {
		if(item_pair.second)
			remove_child(item_pair.second);
	}
	_items.clear();
	do_update(false);
	recalculate_scrollbar();
}

void ListView::do_update(bool dimensions_changed) {
	if(delegate.expired())
		return;
	auto locked_delegate = this->delegate.lock();

	auto preferred_dims = locked_delegate->lv_preferred_item_dimensions();
	_item_dims = {
		_layout == VERTICAL ? current_size().width - 12 : preferred_dims.width,
		preferred_dims.height
	};

	int first = scroll_position().y / _item_dims.height;
	int last = (scroll_position().y + current_size().height) / _item_dims.height;

	if(_layout == GRID) {
		_num_per_row = (current_size().width - 12) / _item_dims.width;
		first *= _num_per_row;
		last = last * _num_per_row + _num_per_row - 1;
	}

	int num = locked_delegate->lv_num_items();
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
		auto rect = item_rect(i);
		if(!_items[i])
			_items[i] = setup_entry(i);
		else if(dimensions_changed)
			_items[i]->set_layout_bounds(rect);
		else
			_items[i]->set_position_nolayout(rect.position());
	}
}

Duck::Ptr<Widget> ListView::setup_entry(int index) {
	if(delegate.expired())
		return nullptr;
	auto locked_delegate = this->delegate.lock();
	auto widget = locked_delegate->lv_create_entry(index);
	add_child(widget);
	widget->set_layout_bounds(item_rect(index));
	return widget;
}

Gfx::Rect ListView::item_rect(int index) {
	return {
		Gfx::Point {(index % _num_per_row) * _item_dims.width, (index / _num_per_row) * _item_dims.height} - scroll_position(),
		_item_dims
	};
}

ListView::ListView(ListView::Layout layout): _layout(layout) {
	set_uses_alpha(true);
}
