/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libui/widget/layout/BoxLayout.h>
#include <libui/Timer.h>
#include <libsys/Process.h>
#include <libui/widget/Label.h>
#include "ProcessMemoryLayoutWidget.h"

class ProcessInspectorWidget: public UI::FlexLayout {
public:
	WIDGET_DEF(ProcessInspectorWidget);

protected:

private:
	ProcessInspectorWidget(const Sys::Process& process);

	void initialize() override;
	void update();

	Sys::Process m_process;
	App::Info m_app_info;
	Duck::Ptr<UI::Timer> m_timer;

	Duck::Ptr<UI::Label> m_pid;
	Duck::Ptr<UI::Label> m_executable;
	Duck::Ptr<UI::Label> m_parent;
	Duck::Ptr<UI::Label> m_group;
	Duck::Ptr<UI::Label> m_user;

	Duck::Ptr<UI::Label> m_memory_phys;
	Duck::Ptr<UI::Label> m_memory_virt;
	Duck::Ptr<UI::Label> m_memory_shared;
	Duck::Ptr<ProcessMemoryLayoutWidget> m_memory_layout;
};
