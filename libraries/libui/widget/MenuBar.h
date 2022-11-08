/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "layout/BoxLayout.h"
#include "../Menu.h"

namespace UI {
	class MenuBar : public BoxLayout {
	public:
		WIDGET_DEF(MenuBar)

		void set_menu(Duck::Ptr<Menu> menu);

	private:
		MenuBar(Duck::Ptr<Menu> menu);

		Duck::Ptr<Menu> m_menu;
	};
}