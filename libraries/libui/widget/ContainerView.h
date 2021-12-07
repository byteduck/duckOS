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

#ifndef DUCKOS_LIBUI_CONTAINERVIEW_H
#define DUCKOS_LIBUI_CONTAINERVIEW_H

#include "ScrollView.h"

namespace UI {
	class ContainerView : public ScrollView {
	public:
		WIDGET_DEF(ContainerView)

		//ScrollView
		void on_scroll(Point new_position) override;
		Dimensions scrollable_area() override;
		void set_contents(Widget::ArgPtr contents);

		//Widget
		void calculate_layout() override;
		Dimensions preferred_size() override;

	private:
		explicit ContainerView();

		Widget::Ptr _contents = nullptr;
	};
}

#endif //DUCKOS_LIBUI_CONTAINERVIEW_H
