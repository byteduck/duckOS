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

#ifndef DUCKOS_LIBUI_WINDOW_H
#define DUCKOS_LIBUI_WINDOW_H

#include "Widget.h"
#include <libgraphics/geometry.h>
#include <libpond/Window.h>
#include <string>
#include <functional>
#include <libpond/Event.h>

namespace UI {
	class Window {
	public:
		static Window* create();

		void resize(int width, int height);
		int width();
		int height();

		void set_position(int x, int y);
		int x_position();
		int y_position();

		void set_contents(Widget* contents);
		Widget* contents();

		void set_title(const std::string& title);
		std::string title();

		void repaint();

		Pond::Window* pond_window();

		void on_keyboard(Pond::KeyEvent evt);
		void on_mouse_move(Pond::MouseMoveEvent evt);
		void on_mouse_button(Pond::MouseButtonEvent evt);
		void on_mouse_leave(Pond::MouseLeaveEvent evt);

	protected:
		Window();

	private:
		friend class Widget;
		Pond::Window* _window;
		Widget* _contents;
		std::string _title;
	};
}

#endif //DUCKOS_LIBUI_WINDOW_H
