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

#include <kernel/kstd/type_traits.h>

namespace kstd {
	template <typename T>
	typename remove_reference<T>::type&& move(T&& arg)
	{
		return static_cast<typename remove_reference<T>::type&&>(arg);
	}

	template<typename T> void swap(T& t1, T& t2) {
		T temp = kstd::move(t1);
		t1 = kstd::move(t2);
		t2 = kstd::move(temp);
	}
}

