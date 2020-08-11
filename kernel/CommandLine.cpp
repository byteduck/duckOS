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
    
    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <kernel/memory/PageDirectory.h>
#include <common/vector.hpp>
#include "CommandLine.h"

namespace CommandLine {
	DC::string cmdline;
	DC::vector<Option> options;

	void init(const struct multiboot_info& header) {
		if(header.flags & MULTIBOOT_INFO_CMDLINE) {
			char* str = (char*) PageDirectory::k_mmap(header.cmdline, 1024, true);
			if(!str) {
				printf("[kinit] cmdline couldn't be mmap-ed!\n");
				return;
			}
			cmdline = str;
			PageDirectory::k_munmap(str);

			printf("[kinit] Command line options: '%s'\n", cmdline.c_str());

			DC::string cmd = cmdline;
			DC::string part = "";
			while(cmd.length()) {
				size_t space_index = cmd.find(' ');
				part = (int) space_index != -1 ? cmd.substr(0, space_index) : cmd;
				cmd = (int) space_index != -1 ? cmd.substr(space_index + 1, cmd.length()) : "";
				if(!part.length()) continue;
				size_t equal_index = part.find('=');
				if((int) equal_index != -1)
					options.push_back({
						part.substr(0, equal_index),
						part.substr(equal_index + 1, part.length())
					});
				else
					options.push_back({
						part,
						""
					});
			}
		}
	}

	DC::string get_option_value(char* name) {
		for(size_t i = 0; i < options.size(); i++) {
			if(options[i].name == name) return options[i].value;
		}
		return "";
	}

	bool has_option(char* name) {
		for(size_t i = 0; i < options.size(); i++) {
			if(options[i].name == name) return true;
		}
		return false;
	}
}