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

#include <cstring>
#include <iostream>
#include "ConfigFile.h"
#include <libduck/StringUtils.h>

using namespace Duck;

ConfigFile::ConfigFile(const std::string& filename) {
	try {
		_file.open(filename, std::ios::in);
	} catch(const std::ifstream::failure& e) {
		fprintf(stderr, "ConfigFile(%s) failed to open: %s\n", filename.c_str(), e.what());
	}
}

ConfigFile::~ConfigFile() {
	if(_file.is_open())
		_file.close();
}

bool ConfigFile::read() {
	if(!_file.is_open())
		return false;

	std::string curr_header = "";
	std::string line;
	while(getline(_file, line)) {
		trim(line);
		if(line[0] == '[' && line[line.length() - 1] == ']') {
			curr_header = line.substr(1, line.length() - 2);
			continue;
		}

		auto eq_index = line.find_first_of('=');
		if(eq_index != std::string::npos) {
			auto key = line.substr(0, eq_index);
			rtrim(key);

			auto val = line.substr(eq_index + 1);
			ltrim(val);

			if(val[0] == '"' && val[val.length() - 1] == '"')
				val = val.substr(1, val.length() - 2);

			_values[curr_header][key] = val;
		}
	}

	_file.close();
	return true;
}

void ConfigFile::close() {
	if(_file.is_open())
		return _file.close();
}

std::map<std::string, std::string>& ConfigFile::operator[](const std::string& name) {
	return _values[name];
}

std::map<std::string, std::string>& ConfigFile::section(const std::string& name) {
	return _values[name];
}

bool ConfigFile::has_section(const std::string& name) {
	return _values.find(name) != _values.end();
}

std::map<std::string, std::string>& ConfigFile::defaults() {
	return _values[""];
}

