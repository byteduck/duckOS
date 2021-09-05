/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#ifndef DUCKOS_LIBAPP_APP_H
#define DUCKOS_LIBAPP_APP_H

#include <vector>
#include <string>
#include <libgraphics/Image.h>
#include <libduck/Result.hpp>
#include <memory>

#define LIBAPP_CONFIG_BASEPATH "/usr/share/applications/"
#define LIBAPP_ICON_BASEPATH "/usr/share/icons/16x16/"
#define LIBAPP_MISSING_ICON "/usr/share/icons/16x16/missing_icon.png"

namespace App {
	class Info {
	public:
		static ResultRet<Info> from_config_file(const std::string& config_file);
		static ResultRet<Info> from_app_name(const std::string& app_name);

		Info() = default;
		Info(std::string name, std::string icon_name, std::string exec);

		const Gfx::Image& icon();
		const std::string& name() const;
		const std::string& icon_name() const;
		const std::string& exec() const;
		bool exists() const;

	private:
		bool _exists = false;
		std::string _name;
		std::string _icon_name;
		std::string _exec;
		std::shared_ptr<Gfx::Image> _icon = nullptr;
	};


	std::vector<Info> get_all_apps();
}

#endif //DUCKOS_LIBAPP_APP_H