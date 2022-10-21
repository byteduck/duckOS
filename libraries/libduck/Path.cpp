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

#include "Path.h"
#include "DirectoryEntry.h"

using namespace Duck;

Path::Path(std::string string): m_is_absolute(string[0] == '/') {
	//If the string is empty, then use "." as the path.
	if(string.empty())
		string = ".";

	//Split the string into parts
	std::vector<std::string> temp_parts;
    std::size_t end;
    do {
		end = string.find('/');
		auto part = string.substr(0, end);
		if(end != std::string::npos)
			string.erase(0, end + 1);

		if(part.empty() || part == ".") {
			//If the part is . or empty, (AKA current directory), don't add it
			continue;
		} else if(part == ".." && !temp_parts.empty() && temp_parts.back() != "..") {
			//If the part is .. (AKA previous directory) and we have a previous part, remove the last part
			temp_parts.pop_back();
			continue;
		}

        temp_parts.push_back(part);
    } while(end != std::string::npos);

	//If we have no parts, then we should just use "."
	if(temp_parts.empty() && !m_is_absolute) 
		temp_parts.push_back(".");

	//Then, build a path out of the parts
	if(m_is_absolute)
		m_path = "/";
	for(auto& part : temp_parts)
		m_path += part + (&part != &temp_parts.back() ? "/" : "");

	rebuild_parts();
}

Path::operator std::string() const {
	return m_path;
}

ResultRet<std::vector<DirectoryEntry>> Path::get_directory_entries() const {
	auto* dir = opendir(m_path.c_str());
	if(!dir)
		return Result(errno);

	std::vector<DirectoryEntry> entries;

	struct dirent* entry;
	while((entry = readdir(dir)) != NULL)
		if(std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
			entries.emplace_back(*this, entry);

	closedir(dir);

	return std::move(entries);
}

void Path::rebuild_parts() {
	m_parts.clear();

	//Then, create parts out of that path
	std::string_view path_view(m_path);
	std::size_t end;
    do {
		end = path_view.find('/');
		auto part = path_view.substr(0, end);
		if(end != std::string_view::npos)
			path_view = path_view.substr(end + 1);

		if(part.empty())
			continue;
		
		m_parts.push_back(std::string(part));
	} while(end != std::string_view::npos);

	if(m_parts.empty())
		m_parts.push_back(m_path);

	//Then, figure out the extension and filename of the path
	auto base = basename();
	auto extension_idx = base.find_last_of(".");
	if(extension_idx != 0 && extension_idx != base.size() - 1 && extension_idx != std::string_view::npos) {
		//If the last dot is at the beginning of the filename, that just means it's an extension-less dotfile
		//If the last dot is at the end of the filename, it has no extension.
		m_extension = base.substr(extension_idx + 1);
		m_filename = base.substr(0, extension_idx);
	} else {
		m_filename = base;
	}
}