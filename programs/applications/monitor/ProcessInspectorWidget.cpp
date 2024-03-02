/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "ProcessInspectorWidget.h"
#include "ProcessManager.h"
#include <libui/libui.h>
#include <libui/widget/Image.h>
#include <libui/widget/Label.h>
#include <libui/widget/Cell.h>
#include <libui/widget/NamedCell.h>

ProcessInspectorWidget::ProcessInspectorWidget(const Sys::Process& process):
	UI::FlexLayout(VERTICAL),
	m_process(process)
{
	m_timer = UI::set_interval([this] {
		update();
	}, 1000);

	auto app_info_res = m_process.app_info();
	if (app_info_res.has_value())
		m_app_info = app_info_res.value();
}

void ProcessInspectorWidget::initialize() {
	// Header
	{
		auto header = UI::BoxLayout::make(UI::BoxLayout::HORIZONTAL, 4);
		auto name = m_process.name();
		Duck::Ptr<const Gfx::Image> image;
		if (m_app_info.exists()) {
			image = m_app_info.icon();
			name = m_app_info.name();
		} else {
			image = UI::icon("/filetypes/default");
		}
		header->add_child(UI::Image::make(image, UI::Image::FIT, Gfx::Dimensions { 32, 32 }));
		header->add_child(UI::Label::make(name));
		auto cell = UI::Cell::make(header);
		cell->set_sizing_mode(UI::PREFERRED);
		add_child(cell);
	}

	// Status
	{
		auto stack = UI::BoxLayout::make(UI::BoxLayout::VERTICAL);
		stack->add_child(m_pid = UI::Label::make("PID: " + std::to_string(m_process.pid()), UI::BEGINNING));
		stack->add_child(m_executable = UI::Label::make("Executable: " + m_process.exe(), UI::BEGINNING));
		stack->add_child(m_parent = UI::Label::make("Parent: " + std::to_string(m_process.ppid()), UI::BEGINNING));
		stack->add_child(m_group = UI::Label::make("", UI::BEGINNING));
		stack->add_child(m_user = UI::Label::make("", UI::BEGINNING));
		add_child(UI::NamedCell::make("Status", stack));
	}

	// Memory info
	{
		auto stack = UI::BoxLayout::make(UI::BoxLayout::VERTICAL);
		stack->add_child(m_memory_phys = UI::Label::make("", UI::BEGINNING));
		stack->add_child(m_memory_virt = UI::Label::make("", UI::BEGINNING));
		stack->add_child(m_memory_shared = UI::Label::make("", UI::BEGINNING));
		add_child(UI::NamedCell::make("Memory", stack));
	}

	// Memory layout
	{
		m_memory_layout = ProcessMemoryLayoutWidget::make(m_process);
		auto cell = UI::NamedCell::make("Memory Layout", m_memory_layout);
		cell->set_sizing_mode(UI::FILL);
		add_child(cell);
	}

	update();
}

void ProcessInspectorWidget::update() {
	m_process = ProcessManager::inst().processes().at(m_process.pid());

	m_group->set_label("GID: " + std::to_string(m_process.gid()));
	m_user->set_label("User: " + std::to_string(m_process.uid()));

	m_memory_phys->set_label("Physical: " + m_process.physical_mem().readable());
	m_memory_virt->set_label("Virtual: " + m_process.virtual_mem().readable());
	m_memory_shared->set_label("Shared: " + m_process.shared_mem().readable());
	m_memory_layout->update();
}