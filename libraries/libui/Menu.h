/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <libduck/Serializable.h>
#include "libduck/Object.h"

namespace UI {
	class Menu;

	class MenuItem: public Duck::Object, public Duck::Serializable {
	public:
		DUCK_OBJECT_DEF(MenuItem)

		using Action = std::function<void()>;

		static const Duck::Ptr<MenuItem> Separator;

		// Serializable
		size_t serialized_size() const override;
		void serialize(uint8_t*& buf) const override;
		void deserialize(const uint8_t*& buf) override;

		// MenuItem
		void set_title(std::string title);
		std::string title() const;

		void set_action(Action action);
		Action action() const;

		void set_submenu(Duck::Ptr<Menu> submenu);
		Duck::Ptr<Menu> submenu() const;

	private:
		MenuItem(std::string title = "", Action action = nullptr, Duck::Ptr<Menu> submenu = nullptr);
		explicit MenuItem(const uint8_t*& buf);

		std::string m_title; ///< The title of the menu item.
		Action m_action; ///< The action performed when the item is selected.
		Duck::Ptr<Menu> m_submenu; ///< The submenu for this menu item.
		int m_id;
	};

	class Menu: public Duck::Object, public Duck::Serializable {
	public:
		DUCK_OBJECT_DEF(Menu)

		static Duck::Ptr<Menu> make(std::vector<Duck::Ptr<MenuItem>> items = {});

		/**
		 * Adds an item to the menu.
		 * @param item The menu item to add.
		 */
		void add_item(Duck::Ptr<MenuItem> item);

		/**
		 * Gets the items in the menu.
		 * @return A reference to the list of items in the menu.
		 */
		const std::vector<Duck::Ptr<MenuItem>>& items();

		/**
		 * Gets the number of items in the menu.
		 */
		int size() const;

		// Serializable
		MAKE_SERIALIZABLE(m_items);

	private:
		explicit Menu(std::vector<Duck::Ptr<MenuItem>> items);
		explicit Menu(const uint8_t*& buf);

		std::vector<Duck::Ptr<MenuItem>> m_items;
	};
}
