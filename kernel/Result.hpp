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

#include <kernel/kstd/shared_ptr.hpp>
#include <kernel/kstd/kstdio.h>

class Result {
public:
	static const int Success = 0;

	Result(int code);

	bool is_success() const;
	bool is_error() const;
	int code() const;
private:
	int _code;
};

template<typename T>
class ResultRet {
public:
	ResultRet(int error):  _result(error) {};
	ResultRet(Result error): _result(error) {};
	ResultRet(T ret): _ret(ret), _result(0) {};
	bool is_error() const {return _result.is_error();}
	int code() const {return _result.code();}
	T& value() {
		ASSERT(!is_error());
		return _ret;
	};
private:
	T _ret;
	Result _result;
};

