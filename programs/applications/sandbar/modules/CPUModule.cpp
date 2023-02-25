/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "CPUModule.h"
#include "../Sandbar.h"

CPUModule::CPUModule() {
	m_stream = Duck::FileInputStream("/proc/cpuinfo");
}

float CPUModule::plot_value() {
	auto val = Sys::CPU::get_info(m_stream);
	if(val.has_value())
		return val.value().utilization / 100.0;
	return 0.0;
}
