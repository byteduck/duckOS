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

#include <cassert>
#include <optional>
#include <cstring>

namespace Duck {
	class Result {
	public:
		Result(int code): m_code(code) {}

		[[nodiscard]] bool is_success() const { return m_code == 0; }
		[[nodiscard]] bool is_error() const { return m_code; }
		[[nodiscard]] int code() const { return m_code; }
		[[nodiscard]] const char* strerror() const { return ::strerror(m_code); }
		operator int() const { return m_code; }

		static const Result SUCCESS;
		static const Result FAILURE;

	private:
		int m_code;
	};

	template<typename T>
	class ResultRet {
	public:
		ResultRet(Result error): m_ret(std::nullopt), m_result(error) {};
		ResultRet(std::optional<T> ret): m_ret(std::move(ret)), m_result(0) {};
		ResultRet(T ret): m_ret(std::move(ret)), m_result(0) {};

		[[nodiscard]] bool is_error() const { return m_result.is_error(); }
		[[nodiscard]] bool has_value() const { return m_ret.has_value(); }
		[[nodiscard]] int code() const { return m_result.code(); }
		[[nodiscard]] Result result() const { return m_result; }
		[[nodiscard]] const char* strerror() const { return m_result.strerror(); }
		[[nodiscard]] T& value() { return m_ret.value(); };
		operator T&() { return m_ret.value(); }

	private:
		std::optional<T> m_ret;
		Result m_result;
	};
}