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
    
    Copyright (c) Byteduck 2016-2022. All rights reserved.
*/
#include "Service.h"
#include "libduck/Log.h"
#include <libduck/Config.h>
#include <libduck/StringStream.h>
#include <unistd.h>

Duck::ResultRet<Service> Service::load_service(Duck::Path path) {
	auto config_res = Duck::Config::read_from(path);
	if(config_res.is_error())
		return config_res.result();

	auto& config = config_res.value();
	if(!config.has_section("service"))
		return Duck::Result(-EINVAL);

	auto& service = config["service"];

	return Service(service["name"], service["exec"], service["after"]);
}

std::vector<Service> Service::get_all_services() {
	auto res = Duck::Path("/etc/init/services").get_directory_entries();
	if(res.is_error())
		return {};

	auto ret = std::vector<Service>();
	for(auto& entry : res.value()) {
		if(!entry.is_regular() || entry.path().extension() != "service")
			continue;

		auto service_res = load_service(entry.path());
		if(service_res.is_error()) {
			Duck::Log::warn("Failed to load service at ", entry.path().string(), ": ", service_res.result().strerror());
			continue;
		}

		ret.push_back(service_res.value());
	}

	return std::move(ret);
}

const std::string& Service::name() const {
	return m_name;
}

const std::string& Service::exec() const {
	return m_exec;
}

const std::string& Service::after() const {
	return m_after;
}

void Service::execute() const {
	Duck::Log::info("Starting service ", m_name, "...");
	pid_t pid = fork();

	if(pid == 0) {
		Duck::StringInputStream exec_stream(m_exec);
		exec_stream.set_delimeter(' ');

		//Split arguments from exec command
		std::vector<std::string> args;
		std::string arg;
		while(!exec_stream.eof()) {
			exec_stream >> arg;
			args.push_back(arg);
		}

		//Convert c++ string vector into cstring array
		const char* c_args[args.size() + 1];
		for(auto i = 0; i < args.size(); i++)
			c_args[i] = args[i].c_str();
		c_args[args.size()] = NULL;

		char* env[] = {NULL};

		//Execute the command
		execvpe(c_args[0], (char* const*) c_args, env);
		Duck::Log::err("Failed to execute ", m_exec, ": ", strerror(errno));
		exit(-1);
	}
}

Service::Service(std::string name, std::string exec, std::string after):
	m_name(std::move(name)), m_exec(std::move(exec)), m_after(std::move(after)) {}
