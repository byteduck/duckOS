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

#pragma once

#include "ScrollView.h"

namespace UI {
	class ListViewDelegate {
	public:
		virtual Duck::Ptr<Widget> lv_create_entry(int index) = 0;
		virtual Gfx::Dimensions lv_preferred_item_dimensions() = 0;
		virtual int lv_num_items() = 0;
	};

	class ListView: public ScrollView {
	public:
		WIDGET_DEF(ListView)

		enum Layout {
			VERTICAL,
			GRID
		};

		Duck::WeakPtr<ListViewDelegate> delegate;

		//ListView
		void update_item(int index);
		void update_data();

	protected:
		//Widget
		void calculate_layout() override;

		//ScrollView
		void on_scroll(Gfx::Point scroll_position) override;
		Gfx::Dimensions scrollable_area() override;

	private:
		ListView(Layout layout = VERTICAL);

		void do_update(bool dimensions_changed);
		Duck::Ptr<Widget> setup_entry(int index);
		Gfx::Rect item_rect(int index);

		std::map<int, Duck::Ptr<Widget>> _items;
		int _prev_first_visible = 0;
		int _prev_last_visible = 0;
		Gfx::Dimensions _item_dims = {-1, -1};
		Gfx::Dimensions _min_dims = {-1, -1};
		int _num_per_row = 1;
		Layout _layout;
	};
}

