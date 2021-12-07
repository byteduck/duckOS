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

#ifndef DUCKOS_LIBUI_LISTVIEW_H
#define DUCKOS_LIBUI_LISTVIEW_H

#include "ScrollView.h"

namespace UI {
	class ListView: public ScrollView {
	public:
		VIRTUAL_WIDGET_DEF(ListView)

		enum Layout {
			VERTICAL,
			GRID
		};

	protected:
		//Widget
		void calculate_layout() override;

		//ScrollView
		void on_scroll(Point scroll_position) override;
		Dimensions scrollable_area() override;

		//ListView
		virtual Widget::Ptr create_entry(int index) = 0;
		virtual Dimensions preferred_item_dimensions() = 0;
		virtual int num_items() = 0;
		void update_item(int index);
		void update_data();

		ListView(Layout layout = VERTICAL);

	private:
		void do_update(bool dimensions_changed);
		Widget::Ptr setup_entry(int index);
		Rect item_rect(int index);

		std::map<int, Widget::Ptr> _items;
		int _prev_first_visible = 0;
		int _prev_last_visible = 0;
		Dimensions _item_dims = {-1, -1};
		int _num_per_row = 1;
		Layout _layout;
	};
}

#endif //DUCKOS_LIBUI_LISTVIEW_H
