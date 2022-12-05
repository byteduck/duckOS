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
#include "TimeManager.h"

Time::Time(): _sec(0), _usec(0) {}

Time::Time(long sec, long usec): _sec(sec), _usec(usec) {}

Time::Time(timespec spec): _sec(spec.tv_sec), _usec(spec.tv_usec) {
	_sec += _usec / 1000000;
	_usec %= 1000000;
}

Time Time::now() {
	return Time(TimeManager::now());
}

Time Time::distant_future() {
	return {2147483647L, 0};
}

timespec Time::to_timespec() const {
	return {_sec, _usec};
}

timeval Time::to_timeval() const {
	return {_sec, _usec};
}

long Time::sec() const {
	return _sec;
}

long Time::usec() const {
	return _usec;
}

Time Time::operator+(const Time& other) const {
	Time ret(to_timespec());
	ret._usec += other._usec;
	ret._sec += other._sec + ret._usec / 1000000;
	ret._usec %= 1000000;
	return ret;
}


Time Time::operator- (const Time& other) const {
	Time ret(to_timespec());
	ret._sec -= other._sec;
	ret._usec -= other._usec;
	if(ret._usec < 0) {
		ret._sec -= 1 + ret._usec / -1000000;
		ret._usec = (1000000 - (-ret._usec % 1000000)) % 1000000;
	}
	return ret;
}

bool Time::operator>(const Time& other) const {
	return _sec > other._sec || (_sec == other._sec && _usec > other._usec);
}

bool Time::operator>=(const Time& other) const {
	return _sec > other._sec || (_sec == other._sec && _usec >= other._usec);
}

bool Time::operator<(const Time& other) const {
	return _sec < other._sec || (_sec == other._sec && _usec < other._usec);
}

bool Time::operator<=(const Time& other) const {
	return _sec < other._sec || (_sec == other._sec && _usec <= other._usec);
}

bool Time::operator==(const Time& other) const {
	return _sec == other._sec && _usec == other._usec;
}
