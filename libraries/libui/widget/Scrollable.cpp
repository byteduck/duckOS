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

#include "Scrollable.h"
#include "ScrollView.h"

std::shared_ptr<UI::ScrollView> UI::Scrollable::scroll_view() {
    return _scroll_view;
}

void UI::Scrollable::set_scrollview(const UI::ScrollView::Ptr& view) {
    _scroll_view = view;
}

Point UI::Scrollable::scroll_position() {
    if(!scroll_view())
        return {0, 0};
    return scroll_view()->scroll_position();
}

bool UI::Scrollable::needs_layout_on_child_change() {
    return false;
}
