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
#include <iostream>

using namespace Sys;

ResultRet<Mem::Info> Mem::get_info(std::istream& file) {
	file.clear(file.goodbit);
	file.seekg(0);
	if(file.fail())
		return Result::FAILURE;

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
		strtoul(cfg["kheap"].c_str(), nullptr, 0)
	};
}

ResultRet<Mem::Info> Mem::get_info() {
	std::ifstream mem_info("/proc/meminfo");
	if(mem_info.fail())
		return Result(Result::FAILURE);
	return get_info(mem_info);
}

#define GiB 1073742000
#define MiB 1048576
#define KiB 1024

double get_human_size(unsigned long bytes) {
	if(bytes > GiB) {
		return bytes / (double) GiB;
	} else if(bytes > MiB) {
		return bytes / (double) MiB;
	} else if(bytes > KiB) {
		return bytes / (double) KiB;
	} else return bytes;
}

const char* get_human_suffix(unsigned long bytes) {
	if(bytes > GiB) {
		return "GiB";
	} else if(bytes > MiB) {
		return "MiB";
	} else if(bytes > KiB) {
		return "KiB";
	} else return "bytes";
}

std::string Mem::Amount::readable() const {
	char buf[512];
	snprintf(buf, 512, "%.2f %s", get_human_size(bytes), get_human_suffix(bytes));
	return buf;
}
