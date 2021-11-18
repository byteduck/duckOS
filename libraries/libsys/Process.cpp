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

#include "Process.h"
#include <filesystem>
#include <fstream>
#include <libduck/Config.h>
#include <unistd.h>

using namespace Sys;

std::map<pid_t, Process> Process::get_all() {
	std::map<pid_t, Process> ret;
	for(const auto& ent : std::filesystem::directory_iterator("/proc/")) {
		if(ent.is_directory() && std::isdigit(ent.path().filename().string()[0])) {
			pid_t pid = std::stoi(ent.path().filename());
			auto proc_res = get(pid);
			if(proc_res.has_value())
				ret[pid] = proc_res.value();
		}
	}
	return ret;
}

ResultRet<Process> Process::get(pid_t pid) {
	Process ret;
	ret._pid = pid;
	auto res = ret.update();
	if(res.is_error())
		return res;
	return ret;
}

ResultRet<Process> Process::self() {
	return get(getpid());
}

std::string Process::state_name() const {
	switch(_state) {
		case ALIVE:
			return "Alive";
		case ZOMBIE:
			return "Zombie";
		case DEAD:
			return "Dead";
		case BLOCKED:
			return "Sleeping";
		default:
			return "Unknown";
	}
}

std::string Process::exe() const {
	char link[256];
	link[0] = '\0';
	readlink(("/proc/" + std::to_string(_pid) + "/exe").c_str(), link, 256);
	return link;
}

ResultRet<App::Info> Process::app_info() const {
	return App::Info::from_app_directory(std::filesystem::path(exe()).parent_path());
}

Result Process::update() {
	auto cfg_res = Duck::Config::read_from("/proc/" + std::to_string(_pid) + "/status");
	if(cfg_res.is_error())
		return cfg_res.result();
	auto& cfg = cfg_res.value();

	if(!cfg.has_section("proc"))
		return Result::FAILURE;
	auto& proc = cfg["proc"];

	_name = proc["name"];
	_pid = std::stoi(proc["pid"]);
	_ppid = std::stoi(proc["ppid"]);
	_gid = std::stoi(proc["gid"]);
	_uid = std::stoi(proc["uid"]);
	_state = (State) std::stoi(proc["state"]);
	_physical_mem = {std::stoul(proc["pmem"])};
	_virtual_mem = {std::stoul(proc["vmem"])};
	_shared_mem = {std::stoul(proc["shmem"])};

	return Result::SUCCESS;
}
