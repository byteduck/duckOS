/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace UI {
	class Menu;

	class MenuItem {
	public:
		using Ptr = std::shared_ptr<MenuItem>;
		using ArgPtr = std::shared_ptr<MenuItem>&;
		using Action = std::function<void()>;

		static const Ptr Separator;

		static Ptr make(std::string title = "", Action action = nullptr);
		static Ptr make(std::string title, std::shared_ptr<Menu> submenu);

		std::string title;
		Action action;
		std::shared_ptr<Menu> submenu;

	private:
		MenuItem(std::string title, Action action, std::shared_ptr<Menu> submenu);
	};

	class Menu {
	public:
		using Ptr = std::shared_ptr<Menu>;
		using ArgPtr = std::shared_ptr<Menu>&;

		static Ptr make(std::vector<MenuItem::Ptr> items = {});

		void add_item(MenuItem::ArgPtr item);
		const std::vector<MenuItem::Ptr>& items();
		int size();

	private:
		explicit Menu(std::vector<MenuItem::Ptr> items);

		std::vector<MenuItem::Ptr> m_items;
	};
}
