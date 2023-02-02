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

#include "CalculatorWidget.h"
#include <libui/libui.h>

using namespace UI;

#define NUM_BUTTON(x) \
	auto btn_##x = Button::make(#x); \
	btn_##x->on_pressed = [&] { \
		add_digit(x); \
	}; \
	button_grid->add_child(btn_##x);

#define OP_BUTTON(op, label) \
	auto btn_##op = Button::make(#label); \
	btn_##op->on_pressed = [&] { \
		do_op(#op); \
	}; \
	button_grid->add_child(btn_##op);

CalculatorWidget::CalculatorWidget(): BoxLayout(VERTICAL) {
	display = Label::make("0");
	display->set_alignment(UI::CENTER, UI::END);
	display->set_font(UI::pond_context->get_font("gohu-14"));
	display->set_padding({8, 8});
	add_child(display);

	button_grid = GridLayout::make(Gfx::Dimensions {4, 0});
	add_child(button_grid);

	OP_BUTTON(del, DEL)
	OP_BUTTON(ce, CE)
	OP_BUTTON(c, C)
	OP_BUTTON(sign, +/-)

	NUM_BUTTON(7)
	NUM_BUTTON(8)
	NUM_BUTTON(9)
	OP_BUTTON(div, /)

	NUM_BUTTON(4)
	NUM_BUTTON(5)
	NUM_BUTTON(6)
	OP_BUTTON(mul, x)

	NUM_BUTTON(1)
	NUM_BUTTON(2)
	NUM_BUTTON(3)
	OP_BUTTON(sub, -)

	OP_BUTTON(dec, .)
	NUM_BUTTON(0)
	OP_BUTTON(equ, =)
	OP_BUTTON(add, +)
}

void CalculatorWidget::add_digit(int digit) {
	if(just_hit_op) {
		just_hit_op = false;
		hit_dec = false;
		dec_multiplier = 0.1;
		num = 0;
	}

	if(!hit_dec) {
		num = num * 10 + digit;
	} else {
		num += dec_multiplier * digit;
		dec_multiplier *= 0.1;
	}

	disp_num(num);
}

void CalculatorWidget::do_op(const std::string& op) {
	if(op == "sign") {
		if(just_hit_equ) {
			prev_num *= -1;
			disp_num(prev_num);
		} else {
			num *= -1;
			disp_num(num);
		}
		return;
	} else if(op == "del") {
		num /= 10;
		disp_num(num);
		return;
	} else if(op == "c") {
		reset();
		disp_num(num);
		return;
	} else if(op == "ce") {
		num = 0;
		disp_num(num);
		return;
	} else if(op == "dec") {
		if(just_hit_op)
			add_digit(0);
		hit_dec = true;
		disp_num(num);
		return;
	}

	just_hit_op = true;

	if(op == "equ" || (!cur_op.empty() && !just_hit_equ)) {
		if(cur_op == "add") {
			prev_num += num;
		} else if(cur_op == "sub") {
			prev_num -= num;
		} else if(cur_op == "div") {
			if (num != 0) {
				prev_num /= num;
			} else {
				disp_error();
				reset();
				return;
			}
		} else if(cur_op == "mul") {
			prev_num *= num;
		}

		disp_num(prev_num);
	}

	if(op != "equ") {
		if(!just_hit_equ)
			prev_num = num;
		cur_op = op;
		just_hit_equ = false;
	} else {
		just_hit_equ = true;
	}
}

void CalculatorWidget::disp_error() {
	display->set_label("NaN");
}

void CalculatorWidget::reset() {
	just_hit_equ = false;
	num = 0;
	prev_num = 0;
	cur_op = "";
	hit_dec = false;
}

void CalculatorWidget::disp_num(double disp) {
	if(disp == (int) disp) {
		if(hit_dec)
			display->set_label(std::to_string((int) disp) + ".");
		else
			display->set_label(std::to_string((int) disp));
	} else {
		display->set_label(std::to_string(disp));
	}
}