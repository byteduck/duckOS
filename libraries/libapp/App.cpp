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

#include "App.h"
#include <libgraphics/png.h>
#include <libduck/ConfigFile.h>
#include <filesystem>

using namespace App;

ResultRet<Info> Info::from_config_file(const std::string& config_file) {
	Duck::ConfigFile cfg(config_file);
	if(cfg.read() && cfg.has_section("app")) {
		auto& app_config = cfg["app"];
		auto has_name = app_config.find("name") != app_config.end();
		auto has_exec = app_config.find("exec") != app_config.end();
		if(!has_name || !has_exec)
			return Result(-EINVAL);
		return Info(app_config["name"], app_config["icon"], app_config["exec"]);
	}

	return Result(-EINVAL);
}

ResultRet<Info> Info::from_app_name(const std::string& app_name) {
	return from_config_file(std::string(LIBAPP_CONFIG_BASEPATH) + app_name + ".app");
}

Info::Info(std::string name, std::string icon_name, std::string exec): _exists(true), _name(std::move(name)), _icon_name(std::move(icon_name)), _exec(std::move(exec)) {

}

const Gfx::Image& Info::icon() {
	if(!_icon) {
		if(!_icon_name.empty()) {
			_icon = std::shared_ptr<Gfx::Image>(Gfx::load_png(std::string(LIBAPP_ICON_BASEPATH) + _icon_name + ".png"));
			if(!_icon)
				_icon = std::shared_ptr<Gfx::Image>(Gfx::load_png(LIBAPP_MISSING_ICON));
		} else {
			_icon = std::shared_ptr<Gfx::Image>(Gfx::load_png(LIBAPP_MISSING_ICON));
		}
		if(!_icon)
			_icon = std::make_shared<Gfx::Image>(16, 16);
	}
	return *_icon;
}

const std::string& Info::name() const {
	return _name;
}

const std::string& Info::icon_name() const {
	return _icon_name;
}

const std::string& Info::exec() const {
	return _exec;
}

bool Info::exists() const {
	return _exists;
}

std::vector<Info> App::get_all_apps() {
	std::vector<Info> ret;
	for(const auto& ent : std::filesystem::directory_iterator("/usr/share/applications")) {
		if(ent.is_regular_file()) {
			if(ent.path().extension() == ".app") {
				auto app_res = Info::from_config_file(ent.path());
				if(!app_res.is_error())
					ret.push_back(app_res.value());
			}
		}
	}
	return ret;
}