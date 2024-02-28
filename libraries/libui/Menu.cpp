/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "Menu.h"

using namespace UI;
using namespace Duck;

const Duck::Ptr<MenuItem> MenuItem::Separator = MenuItem::make();

MenuItem::MenuItem(std::string title, Action action, Duck::Ptr<Menu> submenu, Keyboard::Shortcut shortcut):
		m_title(std::move(title)), m_action(std::move(action)), m_submenu(std::move(submenu)), m_shortcut(shortcut)
{
	static int menu_id = 0;
	m_id = ++menu_id;
}

MenuItem::MenuItem():
	MenuItem("", nullptr, nullptr, {}) {}

MenuItem::MenuItem(std::string title, Action action, Keyboard::Shortcut shortcut):
	MenuItem(std::move(title), std::move(action), nullptr, shortcut) {}

MenuItem::MenuItem(std::string title, Duck::Ptr<Menu> submenu):
	MenuItem(std::move(title), nullptr, std::move(submenu), {}) {}

MenuItem::MenuItem(std::string title, std::vector<Duck::Ptr<MenuItem>> submenu):
	MenuItem(std::move(title), nullptr, std::move(UI::Menu::make(std::move(submenu))), {}) {}

size_t MenuItem::serialized_size() const {
	return Serialization::buffer_size(m_title, m_id) + sizeof(bool) + (m_submenu ? m_submenu->serialized_size() : 0);
}

void MenuItem::serialize(uint8_t*& buf) const {
	bool has_submenu = m_submenu.operator bool();
	Serialization::serialize(buf, m_title, m_id, has_submenu);
	if(m_submenu)
		m_submenu->serialize(buf);
}

void MenuItem::deserialize(const uint8_t*& buf) {
	bool has_submenu;
	Serialization::deserialize(buf, m_title, m_id, has_submenu);
	if(has_submenu)
		m_submenu = Menu::make(buf);
}

MenuItem::MenuItem(const uint8_t*& buf) {
	deserialize(buf);
}

void MenuItem::set_title(std::string title) {
	m_title = std::move(title);
}

std::string MenuItem::title() const {
	return m_title;
}

void MenuItem::set_action(MenuItem::Action action) {
	m_action = action;
}

MenuItem::Action MenuItem::action() const {
	return m_action;
}

void MenuItem::set_submenu(Duck::Ptr<Menu> submenu) {
	m_submenu = submenu;
}

Duck::Ptr<Menu> MenuItem::submenu() const {
	return m_submenu;
}

void MenuItem::set_shortcut(Keyboard::Shortcut shortcut) {
	m_shortcut = shortcut;
}

Keyboard::Shortcut MenuItem::shortcut() {
	return m_shortcut;
}

Duck::Ptr<Menu> Menu::make(std::vector<Duck::Ptr<MenuItem>> items) {
	return Duck::Ptr<Menu>(new Menu(std::move(items)));
}

Menu::Menu(std::vector<Duck::Ptr<MenuItem>> items): m_items(std::move(items)) {}

void Menu::add_item(Duck::Ptr<MenuItem> item) {
	m_items.push_back(item);
}

const std::vector<Duck::Ptr<MenuItem>>& Menu::items() {
	return m_items;
}

int Menu::size() const {
	return m_items.size();
}

Menu::Menu(const uint8_t*& buf) {
	deserialize(buf);
}
