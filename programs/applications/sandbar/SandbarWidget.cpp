/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "SandbarWidget.h"
#include "Sandbar.h"
#include "TimeWidget.h"
#include <libui/widget/Cell.h>
#include <libui/libui.h>

using namespace UI;
using namespace Duck;

SandbarWidget::SandbarWidget(Duck::Ptr<AppMenu> app_menu):
	UI::FlexLayout(FlexLayout::HORIZONTAL),
	m_app_menu(app_menu)
{
	m_duck_button = UI::Button::make(UI::icon("/duck"));
	m_duck_button->set_sizing_mode(UI::PREFERRED);
	m_duck_button->on_pressed = [&] {
		m_app_menu->toggle();
	};

	add_child(m_duck_button);
	add_child(UI::Cell::make());
	add_child(TimeWidget::make());
}

void SandbarWidget::do_repaint(const DrawContext& ctx) {
	ctx.fill(ctx.rect(), UI::Theme::bg());
}

Gfx::Dimensions SandbarWidget::preferred_size() {
	return {100, Sandbar::HEIGHT};
}
