/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "MenuBar.h"
#include "Button.h"
#include "MenuWidget.h"
#include <functional>
#include <libkeyboard/Keyboard.h>
#include <libui/libui.h>

using namespace Duck;
using namespace UI;

void MenuBar::set_menu(Duck::Ptr<Menu> menu) {
	m_menu = menu;
	for(auto& button : m_buttons)
		remove_child(button.second);
	m_buttons.clear();
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
		m_buttons[item] = button;
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

bool MenuBar::on_keyboard(Pond::KeyEvent evt) {
	if(!KBD_ISPRESSED(evt)) {
		return false;
	}

	Keyboard::Shortcut shortcut = {(Keyboard::Key) evt.key, (Keyboard::Modifier) evt.modifiers};

	std::function<bool(Duck::Ptr<UI::MenuItem>)> do_item = [&](Duck::Ptr<UI::MenuItem> item) -> bool {
		if (item->submenu()) {
			for (auto& item : item->submenu()->items()) {
				if (do_item(item))
					return true;
			}
		} else if(item->shortcut() == shortcut) {
			if (item->action())
				item->action()();
			return true;
		}
		return false;
	};

	for (auto& item : m_menu->items()) {
		if (do_item(item)) {
			auto button = m_buttons[item];
			if (button) {
				button->set_pressed(true);
				UI::set_timeout([button]() {
					button->set_pressed(false);
				}, 100);
			}
			return true;
		}
	}

	return false;
}
