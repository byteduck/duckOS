/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "SandbarWidget.h"
#include "Sandbar.h"
#include "TimeWidget.h"
#include <libui/widget/Cell.h>
#include <libui/widget/Stack.h>
#include <libui/widget/Image.h>
#include <libui/libui.h>

using namespace UI;
using namespace Duck;

SandbarWidget::SandbarWidget(Duck::Ptr<AppMenu> app_menu):
	m_layout(FlexLayout::make(FlexLayout::HORIZONTAL)),
	m_app_menu(app_menu)
{
	add_child(Cell::make(m_layout));

	auto button_stack = Stack::make(Stack::HORIZONTAL, 4, UI::Image::make("/duck"), UI::Label::make("Applications"));
	m_duck_button = UI::Button::make(Cell::make(button_stack, 2));
	m_duck_button->set_sizing_mode(UI::PREFERRED);
	m_duck_button->on_pressed = [&] {
		m_app_menu->toggle();
	};

	m_layout->add_child(m_duck_button);
	m_layout->add_child(UI::Cell::make());
	m_layout->add_child(TimeWidget::make());
}

void SandbarWidget::do_repaint(const DrawContext& ctx) {
	ctx.fill(ctx.rect(), UI::Theme::bg());
}

Gfx::Dimensions SandbarWidget::preferred_size() {
	return {100, Sandbar::HEIGHT};
}
