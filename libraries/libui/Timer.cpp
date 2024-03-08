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

#include "Timer.h"
#include "libui.h"

using namespace UI;

Timer::Timer() = default;

Timer::Timer(int id, int delay, std::function<void()> call, bool is_interval):
	m_id(id), m_delay(delay), m_call(std::move(call)), m_interval(is_interval)
{
	calculate_trigger_time();
}

Timer::~Timer() {
	UI::remove_timer(m_id);
}

void Timer::calculate_trigger_time() {
	auto now = Duck::Time::now();
	if (!m_trigger_time.epoch())
		m_trigger_time = Duck::Time::now();
	while (m_trigger_time <= now)
		m_trigger_time = m_trigger_time + Duck::Time::millis(m_delay);
}

[[nodiscard]] bool Timer::ready() const {
	return millis_until_ready() <= 0;
}

[[nodiscard]] long Timer::millis_until_ready() const {
	return (m_trigger_time - Duck::Time::now()).millis();
}

void Timer::stop() {
	m_enabled = false;
}

void Timer::start() {
	m_enabled = true;
	calculate_trigger_time();
}
