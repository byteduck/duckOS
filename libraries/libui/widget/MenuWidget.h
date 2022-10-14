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
		void window_focus_changed(PtrRef<Window> window, bool focused) override;

		// MenuWidget
		static Ptr<MenuWidget> open_menu(Menu::ArgPtr menu, Gfx::Point location);

	private:
		explicit MenuWidget(Menu::ArgPtr menu, PtrRef<Window> window);

		//Widget
		void do_repaint(const DrawContext& ctx) override;
		void initialize() override;

		//MenuWidget
		void open_child_window(Menu::Ptr menu, Gfx::Rect item_rect);
		WeakPtr<MenuWidget> root_menu();
		bool any_are_focused();
		void close();

		Menu::Ptr m_menu;
		MenuItem::Ptr m_hovered_item;
		MenuItem::Ptr m_last_hovered_item;
		MenuItem::Ptr m_expanded_item;
		WeakPtr<MenuWidget> m_parent;
		WeakPtr<MenuWidget> m_child_menu;
		WeakPtr<Window> m_window;
	};
}
