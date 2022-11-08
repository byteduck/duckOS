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
		button->set_style(ButtonStyle::FLAT);
		WeakPtr<Button> btn_weak = button;
		button->on_pressed = [btn_weak, item] {
			if(item->submenu()) {
				auto rect = btn_weak.lock()->current_absolute_rect().transform(btn_weak.lock()->root_window()->position());
				Duck::Log::dbgf("Open menu at {}", rect.position() + Gfx::Point { 0, rect.height });
				MenuWidget::open_menu(item->submenu(), rect.position() + Gfx::Point { 0, rect.height });
			} else if(item->action()) {
				item->action()();
			}
		};
		add_child(button);
	}
}

MenuBar::MenuBar(Duck::Ptr<Menu> menu): UI::BoxLayout(UI::BoxLayout::HORIZONTAL) {
	set_menu(menu);
}