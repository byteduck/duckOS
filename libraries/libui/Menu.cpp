/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "Menu.h"

using namespace UI;
using namespace Duck;

const Ptr<MenuItem> MenuItem::Separator = MenuItem::make();

MenuItem::MenuItem(std::string title, Action action, Ptr<Menu> submenu):
	title(std::move(title)), action(std::move(action)), submenu(std::move(submenu))
{
	static int menu_id = 0;
	m_id = ++menu_id;
}

size_t MenuItem::serialized_size() const {
	return Serialization::buffer_size(title, m_id) + sizeof(bool) + (submenu ? submenu->serialized_size() : 0);
}

void MenuItem::serialize(uint8_t*& buf) const {
	bool has_submenu = submenu.operator bool();
	Serialization::serialize(buf, title, m_id, has_submenu);
	if(submenu)
		submenu->serialize(buf);
}

void MenuItem::deserialize(const uint8_t*& buf) {
	bool has_submenu;
	Serialization::deserialize(buf, title, m_id, has_submenu);
	if(has_submenu)
		submenu = Menu::make(buf);
}

MenuItem::MenuItem(const uint8_t*& buf) {
	deserialize(buf);
}

Ptr<Menu> Menu::make(std::vector<Ptr<MenuItem>> items) {
	return Ptr<Menu>(new Menu(std::move(items)));
}

Menu::Menu(std::vector<Ptr<MenuItem>> items): m_items(std::move(items)) {}

void Menu::add_item(Ptr<MenuItem> item) {
	m_items.push_back(item);
}

const std::vector<Ptr<MenuItem>>& Menu::items() {
	return m_items;
}

int Menu::size() {
	return m_items.size();
}

Menu::Menu(const uint8_t*& buf) {
	deserialize(buf);
}
