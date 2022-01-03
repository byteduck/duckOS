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
#include <libsys/Memory.h>
#include <libsys/CPU.h>
#include "ProcessListWidget.h"

#define UPDATE_FREQ 1000

using namespace Sys;

UI::ProgressBar::Ptr mem_bar;
UI::ProgressBar::Ptr cpu_bar;
UI::Label::Ptr mem_label;
UI::Label::Ptr cpu_label;
ProcessListWidget::Ptr proc_list;

Duck::FileInputStream mem_stream;
Duck::FileInputStream cpu_stream;

CPU::Info cpu_info;
Mem::Info mem_info;

void update() {
	static timeval last_update = {0, 0};

	//See if it's time to update
	timeval tv = {0, 0};
	gettimeofday(&tv, nullptr);
	int diff = (int) (((tv.tv_sec - last_update.tv_sec) * 1000000) + (tv.tv_usec - last_update.tv_usec))/1000;
	if(diff < UPDATE_FREQ & last_update.tv_sec != 0)
		return;
	last_update = tv;

	auto cpu_res = CPU::get_info(cpu_stream);
	if(!cpu_res.is_error())
		cpu_info = cpu_res.value();

	auto mem_res = Mem::get_info(mem_stream);
	if(!mem_res.is_error())
		mem_info = mem_res.value();

	//Update widgets
	mem_bar->set_progress(mem_info.used_frac());
	std::string mem_string = "Memory: " + mem_info.used.readable() + " / " + mem_info.usable.readable() + " used";
	mem_label->set_label(mem_string);

	cpu_bar->set_progress(cpu_info.utilization / 100.0);
	cpu_label->set_label("CPU: " + std::to_string(cpu_info.utilization) + "%");

	proc_list->update();
}

int main(int argc, char** argv, char** envp) {
	//Open meminfo and cpuinfo
	auto res = mem_stream.open("/proc/meminfo");
	if(res.is_error()) {
		perror("Failed to open meminfo");
		return res.code();
	}

	res = cpu_stream.open("/proc/cpuinfo");
	if(res.is_error()) {
		perror("Failed to open cpuinfo");
		return res.code();
	}

	//Init libUI
	UI::init(argv, envp);

	//Make window
	auto window = UI::Window::create();
	window->set_title("System Monitor");

	//Make widgets
	mem_bar = UI::ProgressBar::make();
	cpu_bar = UI::ProgressBar::make();
	mem_label = UI::Label::make("Memory");
	cpu_label = UI::Label::make("CPU");

	//Make layout
	auto layout = UI::BoxLayout::make(UI::BoxLayout::VERTICAL, 10);

	auto mem_layout = UI::BoxLayout::make(UI::BoxLayout::VERTICAL);
	layout->add_child(mem_layout);
	mem_layout->add_child(mem_bar);
	mem_layout->add_child(mem_label);

	auto cpu_layout = UI::BoxLayout::make(UI::BoxLayout::VERTICAL);
	layout->add_child(cpu_layout);
	cpu_layout->add_child(cpu_bar);
	cpu_layout->add_child(cpu_label);

	proc_list = ProcessListWidget::make();
	layout->add_child(proc_list);

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