/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "ProcessMemoryLayoutWidget.h"
#include <libui/widget/Label.h>

using namespace Sys;
using namespace UI;

ProcessMemoryLayoutWidget::ProcessMemoryLayoutWidget(Sys::Process process):
	m_process(std::move(process))
	{}

void ProcessMemoryLayoutWidget::update() {
	m_memory_regions = m_process.memory_regions();
	m_table_view->update_data();
}

void ProcessMemoryLayoutWidget::initialize() {
	set_sizing_mode(UI::FILL);
	m_table_view->set_delegate(self());
	add_child(m_table_view);
}

Duck::Ptr<UI::Widget> ProcessMemoryLayoutWidget::tv_create_entry(int row, int col) {
	auto reg = m_memory_regions[row];
	std::string contents;

	switch(col) {
		case Col::Start:
			contents = Duck::format("{#x}", reg.start);
			break;
		case Col::Size:
			contents = reg.size.readable();
			break;
		case Col::Offset:
			contents = Duck::format("{#x}", reg.object_start);
			break;
		case Col::Mode:
			contents = Duck::format("{}{}{}{}",
									reg.shared ? "s" : "p",
									reg.prot.read ? "r" : "-",
									reg.prot.write ? "w" : "-",
									reg.prot.execute ? "x" : "-");
			break;
		case Col::Type:
			contents = reg.type == Sys::Process::MemoryRegion::Inode ? "Inode" : "Anonymous";
			break;
		case Col::Name:
			contents = reg.name;
			break;
		default:
			break;
	}

	return Label::make(contents);
}

std::string ProcessMemoryLayoutWidget::tv_column_name(int col) {
	switch(col) {
	case Col::Start:
		return "Start";
	case Col::Size:
		return "Size";
	case Col::Offset:
		return "Offset";
	case Col::Mode:
		return "Mode";
	case Col::Type:
		return "Type";
	case Col::Name:
		return "Name";
	default:
		return "";
	}
}

int ProcessMemoryLayoutWidget::tv_num_entries() {
	return m_memory_regions.size();
}

int ProcessMemoryLayoutWidget::tv_row_height() {
	return 18;
}

int ProcessMemoryLayoutWidget::tv_column_width(int col) {
	switch(col) {
		case Col::Start:
			return 80;
		case Col::Size:
			return 50;
		case Col::Offset:
			return 100;
		case Col::Mode:
			return 40;
		case Col::Type:
			return 80;
		case Col::Name:
			return -1;
		default:
			return -1;
	}
}

void ProcessMemoryLayoutWidget::tv_selection_changed(const std::set<int>& selected_items) {
	TableViewDelegate::tv_selection_changed(selected_items);
}

Duck::Ptr<UI::Menu> ProcessMemoryLayoutWidget::tv_entry_menu(int row) {
	return TableViewDelegate::tv_entry_menu(row);
}

