/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "MemoryModule.h"

MemoryModule::MemoryModule() {
	m_stream = Duck::FileInputStream("/proc/meminfo");
}

float MemoryModule::plot_value() {
	auto val = Sys::Mem::get_info(m_stream);
	if(val.has_value())
		return val.value().used_frac();
	return 0.0;
}