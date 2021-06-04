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

#ifndef DUCKOS_DSH_UTIL_H
#define DUCKOS_DSH_UTIL_H

#include <string>
#include <vector>
#include <algorithm>

inline std::string& rtrim(std::string& str) {
	str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char c) {
		return !std::isspace(c);
	}).base(), str.end());
	return str;
}

inline std::string& ltrim(std::string& str) {
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char c) {
		return !std::isspace(c);
	}));
	return str;
}

inline std::string& trim(std::string& str) {
	return ltrim(rtrim(str));
}

#endif //DUCKOS_DSH_UTIL_H
