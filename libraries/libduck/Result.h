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

namespace Duck {
	class Result {
	public:
		Result(int code): _code(code) {}

		bool is_success() const {
			return _code == 0;
		}

		bool is_error() const {
			return _code;
		}

		int code() const {
			return _code;
		}

		static const Result SUCCESS;
		static const Result FAILURE;

	private:
		int _code;
	};

	template<typename T>
	class ResultRet {
	public:
		ResultRet(Result error): _ret(std::nullopt), _result(error) {};
		ResultRet(std::optional<T> ret): _ret(std::move(ret)), _result(0) {};
		ResultRet(T ret): _ret(std::move(ret)), _result(0) {};
		bool is_error() const {return _result.is_error();}
		bool has_value() const {return _ret.has_value();}
		int code() const {return _result.code();}
		Result result() const {return _result;}
		T& value() {
			assert(_ret.has_value());
			return _ret.value();
		};
	private:
		std::optional<T> _ret;
		Result _result;
	};
}