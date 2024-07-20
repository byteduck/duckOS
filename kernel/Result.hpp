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

#include <kernel/kstd/Arc.h>
#include <kernel/kstd/kstdio.h>
#include <kernel/kstd/type_traits.h>

#define TRY(expr) \
	({ \
        auto res = (expr); \
        if (res.is_error()) \
            return res.result(); \
        res.value(); \
    })

#define TRYRES(expr) \
	({ \
        auto res = (expr); \
        if (res.is_error()) \
            return res; \
    })

class Result {
public:
	static const Result Success;

	explicit Result(int code);

	bool is_success() const;
	bool is_error() const;
	int code() const;
private:
	int _code;
};

template<typename T>
class ResultRet {
public:
	ResultRet(Result error): _result(error) {};
	ResultRet(T ret): _ret(ret), _result(0) {};
	bool is_error() const {return _result.is_error();}
	int code() const {return _result.code();}
	Result result() const {return _result;}
	T& value() {
		ASSERT(!is_error());
		return _ret;
	};
	const T& value() const {
		return _ret;
	}
private:
	T _ret;
	Result _result;
};

