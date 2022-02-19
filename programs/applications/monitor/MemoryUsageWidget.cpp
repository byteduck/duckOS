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
    
    Copyright (c) Byteduck 2016-2022. All rights reserved.
*/

#include "MemoryUsageWidget.h"
#include <libgraphics/Font.h>

using namespace UI;
using namespace Sys;

void MemoryUsageWidget::update() {
	m_mem_info = Mem::get_info(m_mem_stream);
	repaint();
}

Gfx::Dimensions MemoryUsageWidget::preferred_size() {
	auto mem_text = "Memory: " + m_mem_info.used.readable() + " / " + m_mem_info.usable.readable();
	return {Theme::font()->size_of(mem_text.c_str()).width + 20, Theme::progress_bar_height()};
}

void MemoryUsageWidget::do_repaint(const DrawContext& ctx) {
	ctx.draw_inset_rect({0, 0, ctx.width(), ctx.height()});
	Gfx::Rect bar_area = {2, 2, ctx.width() - 4, ctx.height() - 3};

	int user = (int)(((double) (m_mem_info.used - m_mem_info.kernel_phys) / m_mem_info.usable) * bar_area.width);
	int disk = (int)(((double) m_mem_info.kernel_disk_cache / m_mem_info.usable) * bar_area.width);
	int kernel = (int)(((double) (m_mem_info.kernel_phys - m_mem_info.kernel_disk_cache) / m_mem_info.usable) * bar_area.width);

	ctx.fill({
		bar_area.x,
		bar_area.y,
		kernel,
		bar_area.height
	}, UI::Theme::accent());

	ctx.fill({
		bar_area.x + kernel,
		bar_area.y,
		disk,
		bar_area.height
	}, RGB(219, 112, 147));

	ctx.fill({
		bar_area.x + kernel + disk,
		bar_area.y,
		user,
		bar_area.height
	}, RGB(46,139,87));

	auto mem_text = "Memory: " + m_mem_info.used.readable() + " / " + m_mem_info.usable.readable();
	ctx.draw_text(mem_text.c_str(), bar_area, CENTER, CENTER, Theme::font(), Theme::fg());
}

MemoryUsageWidget::MemoryUsageWidget(): m_mem_info({}) {
	auto res = m_mem_stream.open("/proc/meminfo");
	if(res.is_error()) {
		Duck::Log::err("Failed to open meminfo");
		exit(res.code());
	}
}
