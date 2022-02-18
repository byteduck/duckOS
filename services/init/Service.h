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
#pragma once

#include <libduck/Path.h>
#include <libduck/Result.h>

class Service {
public:
	static Duck::ResultRet<Service> load_service(Duck::Path path);
	static std::vector<Service> get_all_services();

	const std::string& name() const;
	const std::string& exec() const;
	const std::string& after() const;

	void execute() const;

private:
	Service(std::string name, std::string exec, std::string after);

	std::string m_name, m_exec, m_after;
};


