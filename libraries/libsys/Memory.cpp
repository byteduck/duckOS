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

#include "Memory.h"
#include <libduck/Config.h>
#include <libduck/FileStream.h>

using namespace Sys;
using Duck::Result, Duck::ResultRet;

ResultRet<Mem::Info> Mem::get_info(Duck::InputStream& file) {
	file.seek(0, Duck::SET);

	auto cfg_res = Duck::Config::read_from(file);
	if(cfg_res.is_error())
		return cfg_res.result();
	if(!cfg_res.value().has_section("mem"))
		return Result(Result::FAILURE);
	auto& cfg = cfg_res.value().section("mem");

	return Mem::Info {
		strtoul(cfg["usable"].c_str(), nullptr, 0),
		strtoul(cfg["used"].c_str(), nullptr, 0),
		strtoul(cfg["reserved"].c_str(), nullptr, 0),
		strtoul(cfg["kvirt"].c_str(), nullptr, 0),
		strtoul(cfg["kphys"].c_str(), nullptr, 0),
		strtoul(cfg["kheap"].c_str(), nullptr, 0),
		strtoul(cfg["kcache"].c_str(), nullptr, 0)
	};
}

ResultRet<Mem::Info> Mem::get_info() {
	auto mem_file = Duck::File::open("/proc/meminfo", "r");
	if(mem_file.is_error())
		return mem_file.result();

	Duck::FileInputStream mem_stream(mem_file);
	return get_info(mem_stream);
}
