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

namespace kstd {
	template< class T > struct remove_reference      {typedef T type;};
	template< class T > struct remove_reference<T&>  {typedef T type;};
	template< class T > struct remove_reference<T&&> {typedef T type;};

	template<class T, T v>
	struct integral_constant {
		static constexpr T value = v;
		using value_type = T;
		using type = integral_constant;
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; }
	};

	using true_type = integral_constant<bool, true>;
	using false_type = integral_constant<bool, false>;

	template<class T, class U>
	struct is_same : false_type {};
	template<class T>
	struct is_same<T, T> : true_type {};
	template<class T, class U>
	inline constexpr bool is_same_v = is_same<T, U>::value;
}

