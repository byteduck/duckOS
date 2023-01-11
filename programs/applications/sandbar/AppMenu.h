/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libui/Window.h>
#include <libui/widget/layout/BoxLayout.h>

class AppMenu: public Duck::Object {
public:
	DUCK_OBJECT_DEF(AppMenu);

	void show();
	void hide();
	void toggle();

	[[nodiscard]] Duck::Ptr<UI::Window> window();

private:
	AppMenu();

	bool m_shown = false;
	Duck::Ptr<UI::Window> m_window;
	Duck::Ptr<UI::BoxLayout> m_layout;
};
