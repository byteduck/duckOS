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

#pragma once

#include <libui/widget/Widget.h>
#include <libsys/Memory.h>
#include <libduck/FileStream.h>

class MemoryUsageWidget: public UI::Widget {
public:
	WIDGET_DEF(MemoryUsageWidget);
	void update(Sys::Mem::Info mem_info);

	Gfx::Dimensions preferred_size() override;

protected:
	void do_repaint(const UI::DrawContext& ctx) override;

private:
	MemoryUsageWidget();

	Sys::Mem::Info m_mem_info;
};


