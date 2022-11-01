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

#pragma once

#include <vector>
#include <string>
#include <libgraphics/Image.h>
#include <libduck/Result.h>
#include <memory>
#include <libduck/Path.h>
#include <map>
#include <set>

#define LIBAPP_BASEPATH "/apps"
#define LIBAPP_MISSING_ICON "/usr/share/icons/missing_icon.icon/16x16.png"

namespace App {
	class Info: public Duck::Serializable {
	public:
		static Duck::ResultRet<Info> from_app_directory(const Duck::Path& app_directory);
		static Duck::ResultRet<Info> from_app_name(const std::string& app_name);
		static Duck::ResultRet<Info> from_current_app();

		Info() = default;
		Info(Duck::Path app_directory, std::string name, std::string exec);

		[[nodiscard]] Duck::Ptr<const Gfx::Image> icon();
		[[nodiscard]] const std::string& name() const;
		[[nodiscard]] const std::string exec() const;
		Duck::Result run(const std::vector<std::string>& args = {}, bool fork = true) const;
		[[nodiscard]] bool exists() const;
		[[nodiscard]] bool hidden() const;
		[[nodiscard]] Duck::Path base_path() const;
		[[nodiscard]] std::set<std::string> extensions() const;
		[[nodiscard]] bool can_handle(Duck::Path path) const;

		[[nodiscard]] Duck::Path resource_path(const Duck::Path& path) const;
		[[nodiscard]] std::shared_ptr<const Gfx::Image> resource_image(const Duck::Path& path);

		///Serializable
		size_t serialized_size() const override;
		void serialize(uint8_t*& buf) const override;
		void deserialize(const uint8_t*& buf) override;

	private:
		bool _exists = false;
		Duck::Path _base_path;
		std::string _name;
		std::string _exec;
		std::set<std::string> _extensions;
		bool _hidden = false;
		std::shared_ptr<Gfx::Image> _icon = nullptr;
		std::map<std::string, std::shared_ptr<Gfx::Image>> _images;
	};


	std::vector<Info> get_all_apps();
	Duck::ResultRet<Info> app_for_file(Duck::Path file);
	Duck::Result open(Duck::Path file, bool fork = true);
}

