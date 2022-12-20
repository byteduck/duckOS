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
#include "Menu.h"
#include <libgraphics/Geometry.h>
#include <libpond/Window.h>
#include <string>
#include <functional>
#include <libpond/Event.h>

namespace UI {
	class WindowDelegate;

	class Window: public Duck::Object {
	public:
		DUCK_OBJECT_DEF(Window)

		///Getters and setters
		void resize(Gfx::Dimensions dims);
		Gfx::Dimensions dimensions();
		Gfx::Rect contents_rect();
		Gfx::Rect accessory_rect();
		bool has_accessory();
		void set_position(Gfx::Point pos);
		Gfx::Point position();
		void set_contents(const std::shared_ptr<Widget>& contents);
		void set_titlebar_accessory(Duck::Ptr<Widget> accessory);
		std::shared_ptr<Widget> contents();
		void set_title(const std::string& title);
		std::string title();
		void set_resizable(bool resizable);
		bool resizable();
		bool is_focused();
		bool is_closed();

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
		void on_resize(const Gfx::Rect& old_rect);
		void on_focus(bool focused);

		///Menus and stuff
		void open_menu(Duck::Ptr<UI::Menu> menu);
		void open_menu(Duck::Ptr<UI::Menu> menu, Gfx::Point point);

		//UI
		void calculate_layout();

		std::weak_ptr<WindowDelegate> delegate;

	protected:
		Window();

	private:
		void initialize() override;
		void blit_widget(Duck::PtrRef<Widget> widget);
		void set_focused_widget(Duck::PtrRef<Widget> widget);

		friend class Widget;
		Pond::Window* _window;
		Duck::Ptr<Widget> _contents;
		Duck::Ptr<Widget> _titlebar_accessory;
		Duck::Ptr<Widget> _focused_widget;
		std::vector<Duck::Ptr<Widget>> _widgets;
		std::string _title;
		Gfx::Point _mouse;
		Gfx::Point _abs_mouse;
		bool _decorated = true;
		bool _uses_alpha = false;
		bool _resizable = false;
		bool _needs_repaint = false;
		bool _focused = false;
		bool _closed = false;
		bool _center_on_show = true;

		struct TitleButton {
			std::string image;
			bool pressed = false;
			Gfx::Rect area = {0,0,0,0};
		};
		TitleButton _close_button = {"win-close"};

	};

	class WindowDelegate {
	public:
		virtual void window_focus_changed(Duck::PtrRef<Window> window, bool focused) = 0;
		virtual ~WindowDelegate() = default;
	};
}

