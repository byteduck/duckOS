/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libui/Window.h>
#include "SandbarWidget.h"

class Sandbar: public Duck::Object {
public:
	DUCK_OBJECT_DEF(Sandbar);

	static constexpr int HEIGHT = 36;

private:
	Sandbar();

	Duck::Ptr<UI::Window> m_window;
	Duck::Ptr<SandbarWidget> m_widget;
	Duck::Ptr<AppMenu> m_app_menu;
};
