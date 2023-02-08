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

#include "../Widget.h"

namespace UI {
	class BoxLayout: public Widget {
	public:
		WIDGET_DEF(BoxLayout)

		enum Direction {
			HORIZONTAL,
			VERTICAL
		};

		void set_spacing(int new_spacing);

		//Widget
		virtual Gfx::Dimensions preferred_size() override;
		virtual Gfx::Dimensions minimum_size() override;
		void calculate_layout() override;

	protected:
		explicit BoxLayout(Direction direction, int spacing = 0);
		int spacing = 0;
		Direction direction;

		Gfx::Dimensions calculate_total_dimensions(std::function<Gfx::Dimensions(Duck::Ptr<Widget>)> dim_func);
	};
}

