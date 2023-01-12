/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libui/widget/layout/FlexLayout.h>
#include <libui/widget/Button.h>
#include "AppMenu.h"

class SandbarWidget: public UI::Widget {
public:
	WIDGET_DEF(SandbarWidget);

protected:
	void do_repaint(const UI::DrawContext& ctx) override;
	Gfx::Dimensions preferred_size() override;

private:
	SandbarWidget(Duck::Ptr<AppMenu> sandbar);

	Duck::Ptr<UI::FlexLayout> m_layout;
	Duck::Ptr<UI::Button> m_duck_button;
	Duck::Ptr<AppMenu> m_app_menu;
};
