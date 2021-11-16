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

#ifndef DUCKOS_LIBUI_SCROLLABLE_H
#define DUCKOS_LIBUI_SCROLLABLE_H

#include "Widget.h"

namespace UI {
    class ScrollView;

    class Scrollable : public Widget {
    public:
        VIRTUAL_WIDGET_DEF(Scrollable)
        virtual void on_scroll(Point new_position) = 0;
        virtual Dimensions scrollable_area() = 0;

    protected:
        std::shared_ptr<ScrollView> scroll_view();
        Point scroll_position();

    private:
        friend class ScrollView;

        void set_scrollview(const std::shared_ptr<ScrollView>& view);

        std::shared_ptr<ScrollView> _scroll_view;
    };
}

#endif //DUCKOS_LIBUI_SCROLLABLE_H
