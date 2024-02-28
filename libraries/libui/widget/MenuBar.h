/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "layout/BoxLayout.h"
#include "../Menu.h"
#include "MenuWidget.h"
#include "Button.h"

namespace UI {
	class MenuBar : public BoxLayout {
	public:
		WIDGET_DEF(MenuBar)

		void set_menu(Duck::Ptr<Menu> menu);

		bool on_keyboard(Pond::KeyEvent evt) override;

	private:
		MenuBar(Duck::Ptr<Menu> menu);
		void on_button_pressed(Duck::Ptr<UI::Button> button, Duck::Ptr<UI::MenuItem> item);

		std::map<Duck::Ptr<MenuItem>, Duck::Ptr<Button>> m_buttons;
		Duck::Ptr<Menu> m_menu;
		Duck::Ptr<MenuWidget> m_opened_menu;
	};
}