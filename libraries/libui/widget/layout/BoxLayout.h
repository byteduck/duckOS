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

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#ifndef DUCKOS_LIBUI_BOXLAYOUT_H
#define DUCKOS_LIBUI_BOXLAYOUT_H

#include "libui/widget/Widget.h"

namespace UI {
	class BoxLayout: public Widget {
	public:
		enum Direction {
			HORIZONTAL,
			VERTICAL
		};

		explicit BoxLayout(Direction direction, int spacing = 0);

		void set_spacing(int new_spacing);

		//Widget
		virtual Dimensions preferred_size() override;
		virtual Rect bounds_for_child(Widget* child) override;

	protected:
		//Widget
		virtual void do_repaint(const DrawContext& ctx) override;

		int spacing = 0;
		Direction direction;
	};
}

#endif //DUCKOS_LIBUI_BOXLAYOUT_H
