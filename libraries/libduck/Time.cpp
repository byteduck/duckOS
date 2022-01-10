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

#include "Time.h"

using namespace Duck;

Time::Time(): m_sec(0), m_usec(0) {}
Time::Time(int64_t sec, long usec): m_sec(sec), m_usec(usec) {}
Time::Time(timeval val): m_sec(val.tv_sec), m_usec(val.tv_usec) {}

Time Time::now() {
	timeval tv;
	gettimeofday(&tv, nullptr);
	return Time {tv};
}


Time Time::operator+(const Time& other) const {
	Time ret(*this);
	ret.m_usec += other.m_usec;
	ret.m_sec += other.m_sec + ret.m_usec / 1000000;
	ret.m_usec %= 1000000;
	return ret;
}


Time Time::operator- (const Time& other) const {
	Time ret(*this);
	ret.m_sec -= other.m_sec;
	ret.m_usec -= other.m_usec;
	if(ret.m_usec < 0) {
		ret.m_sec -= 1 + ret.m_usec / -1000000;
		ret.m_usec = (1000000 - (-ret.m_usec % 1000000)) % 1000000;
	}
	return ret;
}

bool Time::operator>(const Time& other) const {
	return m_sec > other.m_sec || (m_sec == other.m_sec && m_usec > other.m_usec);
}

bool Time::operator>=(const Time& other) const {
	return m_sec > other.m_sec || (m_sec == other.m_sec && m_usec >= other.m_usec);
}

bool Time::operator<(const Time& other) const {
	return m_sec < other.m_sec || (m_sec == other.m_sec && m_usec < other.m_usec);
}

bool Time::operator<=(const Time& other) const {
	return m_sec < other.m_sec || (m_sec == other.m_sec && m_usec <= other.m_usec);
}

bool Time::operator==(const Time& other) const {
	return m_sec == other.m_sec && m_usec == other.m_usec;
}