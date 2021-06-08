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

#include "Line.h"

using namespace Term;

Line::Line() = default;

Line::Line(int size): chars(size) {

}

Character& Line::at(int index) {
	return chars[index];
}

Character& Line::operator[](int index) {
	return chars[index];
}

int Line::length() {
	return chars.size();
}

void Line::resize(int new_size) {
	int old_size = chars.size();
	Character fill_char = {0, old_size ? chars[old_size - 1].attr : Attribute{}};
	chars.resize(new_size);
	for(int i = old_size; i < new_size; i++)
		chars[i] = fill_char;
}

void Line::fill(Character fill_char) {
	for(int i = 0; i < chars.size(); i++)
		chars[i] = fill_char;
}

void Line::clear(Attribute attr) {
	for(int i = 0; i < chars.size(); i++)
		chars[i] = {0, attr};
}