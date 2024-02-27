/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "Widget.h"
#include "../Menu.h"
#include "../Window.h"

namespace UI {
	class MenuWidget: public Widget, public WindowDelegate {
	public:
		WIDGET_DEF(MenuWidget)
		~MenuWidget() override = default;

		// Widget
		virtual bool on_mouse_move(Pond::MouseMoveEvent evt) override;
		virtual bool on_mouse_button(Pond::MouseButtonEvent evt) override;
		virtual void on_mouse_leave(Pond::MouseLeaveEvent evt) override;
		virtual Gfx::Dimensions preferred_size() override;

		// WindowDelegate
		void window_focus_changed(Duck::PtrRef<Window> window, bool focused) override;

		// MenuWidget
		static Duck::Ptr<MenuWidget> open_menu(Duck::Ptr<Menu> menu, Gfx::Point location);
		void close();

		std::function<void()> on_close = nullptr;
	private:
		explicit MenuWidget(Duck::Ptr<Menu> menu, Duck::PtrRef<Window> window);

		//Widget
		void do_repaint(const DrawContext& ctx) override;
		void initialize() override;

		//MenuWidget
		void open_child_window(Duck::Ptr<Menu> menu, Gfx::Rect item_rect);
		Duck::WeakPtr<MenuWidget> root_menu();
		bool any_are_focused();
		void open();

		//Windows
		static Duck::Ptr<MenuWidget> create_menu(Duck::Ptr<Menu> menu, Gfx::Point location, bool submenu);

		// Window management
		static Duck::Ptr<Window> acquire_menu_window();
		static void release_menu_window(Duck::PtrRef<Window> window);
		static void release_all_menu_windows();

		struct MenuWindow {
			Duck::Ptr<Window> window;
			bool used;
		};
		static std::vector<MenuWindow> s_windows;

		Duck::Ptr<Menu> m_menu;
		Duck::Ptr<MenuItem> m_hovered_item;
		Duck::Ptr<MenuItem> m_last_hovered_item;
		Duck::Ptr<MenuItem> m_expanded_item;
		Duck::WeakPtr<MenuWidget> m_parent;
		Duck::WeakPtr<MenuWidget> m_child_menu;
		Duck::WeakPtr<Window> m_window;
	};
}
