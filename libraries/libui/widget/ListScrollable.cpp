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
    update();
}

void ListScrollable::on_scroll(Point scroll) {
    update();
}

Dimensions ListScrollable::scrollable_area() {
    return {_item_dims.width, _item_dims.height * num_items()};
}

void ListScrollable::update_item(int index) {
    //If the item isn't visible, just return
    if(index < _prev_first_visible || index > _prev_last_visible)
        return;

    //If it is, update it
    if(_items[index]) {
        remove_child(_items[index]);
        _items.erase(index);
    }
    _items[index] = create_entry(index);
    add_child(_items[index]);
    auto pos = Point {0, index * _item_dims.height} - scroll_position();
    _items[index]->set_layout_bounds({pos.x, pos.y, current_size().width, _item_dims.height});
}

void ListScrollable::update() {
    _item_dims = preferred_item_dimensions();
    int first = scroll_position().y / _item_dims.height;
    int last = (scroll_position().y + current_size().height) / _item_dims.height;

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
        if(!_items[i]) {
            _items[i] = create_entry(i);
            add_child(_items[i]);
        }

        auto pos = Point {0, i * _item_dims.height} - scroll_position();
        _items[i]->set_layout_bounds({pos.x, pos.y, current_size().width, _item_dims.height});
    }
}

ListScrollable::ListScrollable() {
}
