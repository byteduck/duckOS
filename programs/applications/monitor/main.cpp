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
#include <sys/time.h>
#include <libsys/Memory.h>
#include <libsys/CPU.h>
#include "ProcessListWidget.h"
#include "MemoryUsageWidget.h"
#include <libui/widget/Cell.h>
#include <libduck/FileStream.h>

#define UPDATE_FREQ 1000

using namespace Sys;

Duck::Ptr<UI::ProgressBar> cpu_bar;
Duck::Ptr<MemoryUsageWidget> mem_widget;
Duck::Ptr<ProcessListWidget> proc_list;

Duck::FileInputStream cpu_stream;

CPU::Info cpu_info;

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

	mem_widget->update();

	cpu_bar->set_progress(cpu_info.utilization / 100.0);
	cpu_bar->set_label("CPU: " + std::to_string(cpu_info.utilization) + "%");

	proc_list->update();
}

int main(int argc, char** argv, char** envp) {
	//Open cpuinfo

	auto res = cpu_stream.open("/proc/cpuinfo");
	if(res.is_error()) {
		perror("Failed to open cpuinfo");
		return res.code();
	}

	//Init libUI
	UI::init(argv, envp);

	//Make window
	auto window = UI::Window::make();
	window->set_title("System Monitor");

	//Make widgets
	mem_widget = MemoryUsageWidget::make();
	cpu_bar = UI::ProgressBar::make();

	//Make layout
	auto layout = UI::BoxLayout::make(UI::BoxLayout::VERTICAL, 0);
	layout->add_child(UI::Cell::make(mem_widget));
	layout->add_child(UI::Cell::make(cpu_bar));

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