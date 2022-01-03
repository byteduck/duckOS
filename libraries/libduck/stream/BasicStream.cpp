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

#include "BasicStream.h"
#include "Filestream.h"

using namespace Duck;

[[maybe_unused]] FileInputStream Stream::std_in(File::std_in);
[[maybe_unused]] FileOutputStream Stream::std_out(File::std_out);
[[maybe_unused]] FileOutputStream Stream::std_err(File::std_err);

[[nodiscard]] Result Stream::status(){
	auto ret = m_err;
	m_err = 0;
	return ret;
}