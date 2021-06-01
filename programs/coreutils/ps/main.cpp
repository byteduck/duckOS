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

#include "main.h"
#include <string>
#include <dirent.h>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char** argv, char** envp) {
	DIR *dir;
	struct dirent *ent;

	std::cout << "PID" << '\t' << "PPID" << '\t' << "CMD" << std::endl;

	if ((dir = opendir ("/proc")) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if(ent->d_type == DT_DIR && ent->d_name[0] >= '0' && ent->d_name[0] <= '9') {
				print_proc_info(ent->d_name);
			}
		}
		closedir (dir);
	} else {
		perror("ps");
		return errno;
	}
	return 0;
}

void print_proc_info(const string& proc) {
	fstream proc_file;
	proc_file.open("/proc/" + proc + "/status", ios::in);
	int pid = -1;
	int ppid = -1;
	string cmd;
	if(proc_file.is_open()) {
		string line;
		while(getline(proc_file, line)) {
			get_value(line, "Pid: ", pid);
			get_value(line, "PPid: ", ppid);
			get_value(line, "Name: ", cmd);
		}
		proc_file.close();
	}

	std::cout << pid << '\t' << ppid << '\t' << cmd << std::endl;
}

void get_value(const std::string& line, const std::string& prefix, int& value) {
	if(!line.compare(0, prefix.size(), prefix))
		value = std::stoi(line.substr(prefix.size()));
}

void get_value(const std::string& line, const std::string& prefix, std::string& value) {
	if(!line.compare(0, prefix.size(), prefix))
		value = line.substr(prefix.size());
}