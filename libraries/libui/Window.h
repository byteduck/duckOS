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

#include "libui/widget/Widget.h"
#include <libgraphics/geometry.h>
#include <libpond/Window.h>
#include <string>
#include <functional>
#include <libpond/Event.h>

#define UI_TITLEBAR_HEIGHT 16
#define UI_WINDOW_BORDER_SIZE 2
#define UI_WINDOW_PADDING 2

namespace UI {
	class Window {
	public:
		static Window* create();

		///Getters and setters
		void resize(Dimensions dims);
		Dimensions dimensions();
		void set_position(Point pos);
		Point position();
		void set_contents(Widget* contents);
		Widget* contents();
		void set_title(const std::string& title);
		std::string title();

		///Window management
		void bring_to_front();
		void repaint();
		void close();
		void show();
		void hide();

		///Pond
		Pond::Window* pond_window();

		///Events
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
		Point _mouse;

		struct TitleButton {
			std::string image;
			bool pressed = false;
			Rect area = {0,0,0,0};
		};
		TitleButton _close_button = {"win-close"};

	};
}

#endif //DUCKOS_LIBUI_WINDOW_H
