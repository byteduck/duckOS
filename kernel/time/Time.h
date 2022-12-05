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

#include <kernel/kstd/kstddef.h>
#include <kernel/kstd/unix_types.h>

class Time {
public:
	Time();
	Time(long sec, long usec);
	explicit Time(timespec spec);
	static Time now();
	static Time distant_future();

	timespec to_timespec() const;
	timeval to_timeval() const;
	long sec() const;
	long usec() const;

	Time operator+ (const Time& other) const;
	Time operator- (const Time& other) const;
	bool operator> (const Time& other) const;
	bool operator>= (const Time& other) const;
	bool operator< (const Time& other) const;
	bool operator<= (const Time& other) const;
	bool operator== (const Time& other) const;

private:
	int64_t _sec;
	long _usec;
};

