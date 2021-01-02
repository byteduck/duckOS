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

#ifndef DUCKOS_LIBUI_BUTTON_H
#define DUCKOS_LIBUI_BUTTON_H

#include "Widget.h"
#include <string>
#include <functional>

#define UI_BUTTON_PADDING 5

namespace UI {
	class Button: public Widget {
	public:
		Button(const std::string& label);

		//Button
		std::string label();
		void set_label(const std::string& new_label);

		//Widget
		virtual bool on_mouse(Pond::MouseEvent evt) override;
		virtual Dimensions preferred_size() override;

		std::function<void()> on_pressed = nullptr;
		std::function<void()> on_released = nullptr;
	private:
		//Widget
		void do_repaint(Image& framebuffer) override;

		std::string _label;
		bool _pressed = false;
	};
}

#endif //DUCKOS_LIBUI_BUTTON_H
