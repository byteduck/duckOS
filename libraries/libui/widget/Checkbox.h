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

#ifndef DUCKOS_LIBUI_CHECKBOX_H
#define DUCKOS_LIBUI_CHECKBOX_H

#include "Widget.h"
#include <string>
#include <functional>

namespace UI {
	class Checkbox: public Widget {
	public:
		WIDGET_DEF(Checkbox)

		//Checkbox
		bool checked();
		void set_checked(bool checked);

		std::string label();
		void set_label(const std::string& new_label);

		//Widget
		virtual bool on_mouse_button(Pond::MouseButtonEvent evt) override;
		virtual Dimensions preferred_size() override;

		std::function<void(bool)> on_change = nullptr;
	private:
		explicit Checkbox();
		explicit Checkbox(std::string label);

		//Widget
		void do_repaint(const DrawContext& ctx) override;

		bool _checked = false;
		std::string _label;
	};
}

#endif //DUCKOS_LIBUI_CHECKBOX_H
