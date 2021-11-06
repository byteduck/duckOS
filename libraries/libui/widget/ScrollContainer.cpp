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

#include "ScrollContainer.h"

using namespace UI;

ScrollContainer::ScrollContainer() {
    set_uses_alpha(true);
}

void ScrollContainer::on_scroll(Point new_position) {
    if(_contents)
        _contents->set_position_nolayout(new_position * -1);
}

Dimensions ScrollContainer::scrollable_area() {
    return _contents->preferred_size();
}

void ScrollContainer::set_contents(const std::shared_ptr<Widget>& contents) {
    if(_contents)
        return;
    _contents = contents;
    add_child(_contents);
    update_layout();
}

void ScrollContainer::calculate_layout() {
    if(_contents) {
        auto size = _contents->preferred_size();
        if(_contents->sizing_mode() == FILL)
            size.width = current_size().width;
        _contents->set_layout_bounds({0, -scroll_view()->scroll_position().y, size.width, size.height});
    }
}

Dimensions ScrollContainer::preferred_size() {
    return _contents->preferred_size();
}