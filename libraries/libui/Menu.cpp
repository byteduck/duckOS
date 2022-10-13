/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "Menu.h"
using namespace UI;

const MenuItem::Ptr MenuItem::Separator = MenuItem::make();

MenuItem::Ptr MenuItem::make(std::string title, Action action) {
	return MenuItem::Ptr(new MenuItem(std::move(title), std::move(action), nullptr));
}

MenuItem::Ptr MenuItem::make(std::string title, std::shared_ptr<Menu> submenu) {
	return MenuItem::Ptr(new MenuItem(std::move(title), nullptr, std::move(submenu)));
}

MenuItem::MenuItem(std::string title, Action action, Menu::Ptr submenu):
	title(std::move(title)), action(std::move(action)), submenu(std::move(submenu)) {}

Menu::Ptr Menu::make(std::vector<MenuItem::Ptr> items) {
	return Menu::Ptr(new Menu(std::move(items)));
}

Menu::Menu(std::vector<MenuItem::Ptr> items): m_items(std::move(items)) {}

void Menu::add_item(std::shared_ptr<MenuItem>& item) {
	m_items.push_back(item);
}

const std::vector<MenuItem::Ptr>& Menu::items() {
	return m_items;
}

int Menu::size() {
	return m_items.size();
}
