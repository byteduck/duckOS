/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "MenuBar.h"
#include "Button.h"
#include "MenuWidget.h"

using namespace Duck;
using namespace UI;

void MenuBar::set_menu(Duck::Ptr<Menu> menu) {
	m_menu = menu;
	auto children_copy = children;
	for(auto& child : children_copy)
		remove_child(child);
	for(auto& item : menu->items()) {
		auto button = UI::Button::make(item->title());
		button->set_style(ButtonStyle::INSET);
		button->set_type(UI::ButtonType::TOGGLE);
		button->set_background(RGBA(50, 50, 50, 150));
		WeakPtr<Button> btn_weak = button;
		button->on_pressed = [btn_weak, item, this] {
			on_button_pressed(btn_weak.lock(), item);
		};
		add_child(button);
	}
}

MenuBar::MenuBar(Duck::Ptr<Menu> menu): UI::BoxLayout(UI::BoxLayout::HORIZONTAL) {
	set_menu(menu);
}

void MenuBar::on_button_pressed(Duck::Ptr<UI::Button> button, Duck::Ptr<UI::MenuItem> item) {
	if (!button->is_pressed()) {
		if (m_opened_menu)
			m_opened_menu->close();
		return;
	}

	if(item->submenu()) {
		auto rect = button->current_absolute_rect().transform(button->root_window()->position());
		m_opened_menu = MenuWidget::open_menu(item->submenu(), rect.position() + Gfx::Point { 0, rect.height });
		m_opened_menu->on_close = [button]() {
			button->set_pressed(false);
		};
	} else if(item->action()) {
		item->action()();
	}
}
