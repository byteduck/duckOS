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

#include "libui/widget/Widget.h"
#include <libgraphics/geometry.h>
#include <libpond/Window.h>
#include <string>
#include <functional>
#include <libpond/Event.h>

#define UI_TITLEBAR_HEIGHT 20
#define UI_WINDOW_BORDER_SIZE 2
#define UI_WINDOW_PADDING 2

namespace UI {
	class Window: public std::enable_shared_from_this<Window> {
	public:
		using Ptr = std::shared_ptr<Window>;
		using ArgPtr = const std::shared_ptr<Window>&;
		static Window::Ptr create();

		///Getters and setters
		void resize(Dimensions dims);
		Dimensions dimensions();
		Rect contents_rect();
		void set_position(Point pos);
		Point position();
		void set_contents(const std::shared_ptr<Widget>& contents);
		std::shared_ptr<Widget> contents();
		void set_title(const std::string& title);
		std::string title();
		void set_resizable(bool resizable);
		bool resizable();

		///Window management
		void bring_to_front();
		void repaint();
		void repaint_now();
		void close();
		void show();
		void hide();
		void resize_to_contents();
		void set_uses_alpha(bool uses_alpha);
		void set_decorated(bool decorated);

		///Pond
		Pond::Window* pond_window();

		///Events
		void on_keyboard(Pond::KeyEvent evt);
		void on_mouse_move(Pond::MouseMoveEvent evt);
		void on_mouse_button(Pond::MouseButtonEvent evt);
		void on_mouse_scroll(Pond::MouseScrollEvent evt);
		void on_mouse_leave(Pond::MouseLeaveEvent evt);
		void on_resize(const Rect& old_rect);

		//UI
		void calculate_layout();

	protected:
		Window();

	private:
		void blit_widget(Widget::ArgPtr widget);
		void set_focused_widget(Widget::ArgPtr widget);

		friend class Widget;
		Pond::Window* _window;
		Widget::Ptr _contents;
		Widget::Ptr _focused_widget;
		std::vector<Widget::Ptr> _widgets;
		std::string _title;
		Point _mouse;
		Point _abs_mouse;
		bool _decorated = true;
		bool _uses_alpha = false;
		bool _resizable = false;
		bool _needs_repaint = false;

		struct TitleButton {
			std::string image;
			bool pressed = false;
			Rect area = {0,0,0,0};
		};
		TitleButton _close_button = {"win-close"};

	};
}

