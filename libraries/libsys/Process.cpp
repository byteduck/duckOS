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
#include <libduck/Filesystem.h>
#include <libduck/Config.h>
#include <unistd.h>
#include <libduck/File.h>

using namespace Sys;
using Duck::Result, Duck::ResultRet, Duck::Path;

std::map<pid_t, Process> Process::get_all() {
	std::map<pid_t, Process> ret;

	auto ent_res = Path("/proc/").get_directory_entries();
	if(ent_res.is_error())
		return ret; //This should not happen

	for(const auto& ent : ent_res.value()) {
		if(ent.is_directory() && std::isdigit(ent.name()[0])) {
			pid_t pid = std::stoi(std::string(ent.name()));
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
			return "Blocked";
		case STOPPED:
			return "Stopped";
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
	return App::Info::from_app_directory(Path(exe()).parent());
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

Duck::ResultRet<std::vector<Process::MemoryRegion>> Process::memory_regions() const {
	auto file = TRY(Duck::File::open("/proc/" + std::to_string(_pid) + "/vmspace", "r"));
	auto stream = Duck::FileInputStream(file);
	std::vector<MemoryRegion> out;
	while (!stream.eof()) {
		std::string line;
		stream >> line;
		if (line.empty())
			continue;
		Duck::StringInputStream line_stream {line};
		line_stream.set_delimeter('\t');
		MemoryRegion reg;
		int idx = 0;
		while (!line_stream.eof() && idx <= 4) {
			std::string part;
			line_stream >> part;
			switch (idx) {
				case 0:
					reg.start = strtoul(part.c_str(), nullptr, 0);
					break;
				case 1:
					reg.size = strtoul(part.c_str(), nullptr, 0);
					break;
				case 2:
					reg.object_start = strtoul(part.c_str(), nullptr, 0);
					break;
				case 3:
					reg.prot.read = part.find('r') != std::string::npos;
					reg.prot.write = part.find('w') != std::string::npos;
					reg.prot.execute = part.find('x') != std::string::npos;
					reg.shared = part.find('s') != std::string::npos;
					reg.type = (part.find('A') != std::string::npos) ? MemoryRegion::Anonymous : MemoryRegion::Inode;
					break;
				case 4:
					reg.name = part;
					break;
			}
			idx++;
		}
		if (idx != 5) {
			Duck::Log::warnf("libsys: Invalid memory region description for {}: {}", _pid, line);
			continue;
		}
		out.push_back(reg);
	}
	out.shrink_to_fit();
	return out;
}
