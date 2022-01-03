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

#include "Config.h"
#include "StringUtils.h"
#include "FileStream.h"

using namespace Duck;

std::map<std::string, std::string>& Config::operator[](const std::string& name) {
	return _values[name];
}

std::map<std::string, std::string>& Config::section(const std::string& name) {
	return _values[name];
}

bool Config::has_section(const std::string& name) {
	return _values.find(name) != _values.end();
}

std::map<std::string, std::string>& Config::defaults() {
	return _values[""];
}

ResultRet<Config> Config::read_from(const Path& filename) {
	auto file_res = File::open(filename, "r");
	if(file_res.is_error())
		return file_res.result();

	FileInputStream stream(file_res.value());
	return read_from(stream);
}

ResultRet<Config> Config::read_from(InputStream& stream) {
	Config ret;
	auto res = read_from(stream, ret);
	if(res.is_error())
		return res;
	return ret;
}

Result Config::read_from(InputStream& stream, Config& config) {
	stream.set_delimeter('\n');
	std::string curr_header = "";
	std::string line;
	while(true) {
		if(stream.eof())
			break;

		stream >> line;

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

			config._values[curr_header][key] = val;
		}
	}

	return Result::SUCCESS;
}

