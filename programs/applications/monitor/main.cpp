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

#include <libui/libui.h>
#include <libui/widget/ProgressBar.h>
#include <libui/widget/Label.h>
#include <libui/widget/layout/BoxLayout.h>
#include <cstring>
#include <sys/time.h>

#define GiB 1073742000
#define MiB 1048576
#define KiB 1024

#define UPDATE_FREQ 250

UI::ProgressBar* mem_bar;
UI::ProgressBar* cpu_bar;
UI::Label* mem_label;
UI::Label* cpu_label;

FILE* meminfo;
FILE* cpuinfo;

int get_human_size(unsigned long bytes) {
	if(bytes > GiB) {
		return bytes / GiB;
	} else if(bytes > MiB) {
		return bytes / MiB;
	} else if(bytes > KiB) {
		return bytes / KiB;
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

void update() {
	static timeval last_update = {0, 0};
	static char buf[1024];
	static long usable, used;
	static double utilization;

	//See if it's time to update
	timeval tv = {0, 0};
	gettimeofday(&tv, nullptr);
	int diff = (int) (((tv.tv_sec - last_update.tv_sec) * 1000000) + (tv.tv_usec - last_update.tv_usec))/1000;
	if(diff < UPDATE_FREQ & last_update.tv_sec != 0)
		return;
	last_update = tv;

	//Read meminfo
	fseek(meminfo, 0, SEEK_SET);
	fread(buf, 1, 1024, meminfo);
	char* key = strtok(buf, ":");
	do {
		char* valuestr = strtok(NULL, "\n");
		long value = strtol(valuestr, NULL, 0);

		if(!strcmp(key, "Usable"))
			usable = value;
		else if(!strcmp(key, "Used"))
			used = value;
	} while((key = strtok(NULL, ":")));

	//Read cpuinfo
	fseek(cpuinfo, 0, SEEK_SET);
	fread(buf, 1, 1024, cpuinfo);
	key = strtok(buf, ":");
	do {
		char* valuestr = strtok(NULL, "%\n");
		double value = strtod(valuestr, NULL);

		if(!strcmp(key, "Utilization"))
			utilization = value;
	} while((key = strtok(NULL, ":")));

	//Update widgets
	auto mem_percent = (double)((long double) used / (long double) usable);
	mem_bar->set_progress(mem_percent);
	std::string mem_string = "Memory: " + std::to_string(get_human_size(used)) + std::string(get_human_suffix(used)) + " / " + std::to_string(get_human_size(usable)) + std::string(get_human_suffix(usable)) + " used";
	mem_label->set_label(mem_string);

	cpu_bar->set_progress(utilization / 100.0);
	cpu_label->set_label("CPU: " + std::to_string(utilization) + "%");
}

int main(int argc, char** argv, char** envp) {
	//Open meminfo and cpuinfo
	meminfo = fopen("/proc/meminfo", "r");
	if(!meminfo) {
		perror("Failed to open meminfo");
		return EXIT_FAILURE;
	}

	cpuinfo = fopen("/proc/cpuinfo", "r");
	if(!cpuinfo) {
		perror("Failed to open cpuinfo");
		return EXIT_FAILURE;
	}

	//Init libUI
	UI::init(argv, envp);
	UI::set_app_name("monitor");

	//Make window
	auto* window = UI::Window::create();
	window->set_title("System Monitor");

	//Make widgets
	mem_bar = new UI::ProgressBar();
	cpu_bar = new UI::ProgressBar();
	mem_label = new UI::Label("Memory");
	cpu_label = new UI::Label("CPU");

	//Make layout
	auto* layout = new UI::BoxLayout(UI::BoxLayout::VERTICAL, 10);

	auto* mem_layout = new UI::BoxLayout(UI::BoxLayout::VERTICAL);
	layout->add_child(mem_layout);
	mem_layout->add_child(mem_bar);
	mem_layout->add_child(mem_label);

	auto* cpu_layout = new UI::BoxLayout(UI::BoxLayout::VERTICAL);
	layout->add_child(cpu_layout);
	cpu_layout->add_child(cpu_bar);
	cpu_layout->add_child(cpu_label);

	//Show window
	update();
	window->set_contents(layout);
	window->set_resizable(true);
	window->show();

	//Loop
	while(!UI::ready_to_exit()) {
		update();
		UI::update(UPDATE_FREQ);
	}
}