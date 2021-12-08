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

#include <fstream>
#include "CPU.h"
#include <libduck/Config.h>

using namespace Sys;
using Duck::Result, Duck::ResultRet;

ResultRet<CPU::Info> CPU::get_info(std::istream& file) {
	file.clear(file.goodbit);
	file.seekg(0);
	if(file.fail())
		return Result::FAILURE;

	auto cfg_res = Duck::Config::read_from(file);
	if(cfg_res.is_error())
		return cfg_res.result();

	auto& cfg = cfg_res.value();
	if(!cfg.has_section("cpu"))
		return Result::FAILURE;

	return CPU::Info {std::stod(cfg["cpu"]["util"])};
}

ResultRet<CPU::Info> CPU::get_info() {
	std::ifstream cpu_file("/proc/cpuinfo");
	if(cpu_file.fail())
		return Result::FAILURE;
	return get_info(cpu_file);
}