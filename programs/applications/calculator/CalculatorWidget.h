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

#pragma once

#include <libui/widget/layout/BoxLayout.h>
#include <libui/widget/layout/GridLayout.h>
#include <libui/widget/Button.h>
#include <libui/widget/Label.h>

class CalculatorWidget: public UI::BoxLayout {
public:
	WIDGET_DEF(CalculatorWidget)

private:
	CalculatorWidget();

	void add_digit(int digit);
	void do_op(const std::string& op);
	void disp_num(double disp);

	UI::GridLayout::Ptr button_grid;
	UI::Label::Ptr display;

	long double prev_num = 0;
	long double num = 0;
	std::string cur_op = "";
	bool just_hit_op = false;
	bool just_hit_equ = false;
	bool hit_dec = false;
	double dec_multiplier = 0.1;
};

