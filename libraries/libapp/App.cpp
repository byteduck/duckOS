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
#include <libgraphics/PNG.h>
#include <libduck/Config.h>
#include <unistd.h>

using namespace App;
using Duck::Result, Duck::ResultRet, Duck::Path;

ResultRet<Info> Info::from_app_directory(const Path& app_directory) {
	auto config_res = Duck::Config::read_from(app_directory / "app.conf");
	if(config_res.is_error())
		return config_res.result();
	auto& cfg = config_res.value();

	if(cfg.has_section("app")) {
		auto& app_config = cfg["app"];
		auto has_name = app_config.find("name") != app_config.end();
		auto has_exec = app_config.find("exec") != app_config.end();
		if(!has_name || !has_exec)
			return Result(EINVAL);
		return Info(app_directory, app_config["name"],  app_config["exec"]);
	}

	return Result(EINVAL);
}

ResultRet<Info> Info::from_app_name(const std::string& app_name) {
	return from_app_directory(Path(LIBAPP_BASEPATH) / (app_name + ".app"));
}

ResultRet<Info> Info::from_current_app() {
	char exe_path[512];
	if(!readlink("/proc/$$/exe", exe_path, 512)) {
		return from_app_directory(Path(exe_path).parent());
	} else {
		return Result(errno);
	}
}

Info::Info(Path base_path, std::string name, std::string exec):
	_exists(true), _base_path(std::move(base_path)), _name(std::move(name)), _exec(std::move(exec)) {}

const Gfx::Image& Info::icon() {
	if(!_icon) {
		_icon = std::shared_ptr<Gfx::Image>(Gfx::load_png(_base_path / "icon" / "16x16.png"));
		if(!_icon)
			_icon = std::shared_ptr<Gfx::Image>(Gfx::load_png(LIBAPP_MISSING_ICON));
		if(!_icon)
			_icon = std::make_shared<Gfx::Image>(16, 16);
	}
	return *_icon;
}

const std::string& Info::name() const {
	return _name;
}

const std::string Info::exec() const {
	if(_exec[0] != '/')
		return _base_path / _exec;
	else
		return _exec;
}

bool Info::exists() const {
	return _exists;
}

Path Info::base_path() const {
	return _base_path;
}

Path Info::resource_path(const Path& path) const {
	return _base_path / path;
}

std::shared_ptr<const Gfx::Image> Info::resource_image(const Path& path) {
	auto it = _images.find(path);
	if(it == _images.end()) {
		auto* img = Gfx::load_png(resource_path(path));
		if(img)
			_images[path.string()] = std::shared_ptr<Gfx::Image>(img);
		else
			return std::shared_ptr<Gfx::Image>(nullptr);
	}
	
	return _images[path.string()];
}

std::vector<Info> App::get_all_apps() {
	std::vector<Info> ret;
	auto ent_res = Path(LIBAPP_BASEPATH).get_directory_entries();
	if(ent_res.is_error())
		return ret;
	for(const auto& ent : ent_res.value()) {
		if(ent.is_directory()) {
			if(ent.path().extension() == "app") {
				auto app_res = Info::from_app_directory(ent.path());
				if(!app_res.is_error())
					ret.push_back(app_res.value());
			}
		}
	}
	return ret;
}