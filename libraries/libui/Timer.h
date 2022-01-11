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

#include <functional>
#include <utility>
#include <libduck/Time.h>

namespace UI {
	class Timer {
	public:
		Timer();
		Timer(int id, int delay, std::function<void()> call, bool is_interval);
		~Timer();

		void calculate_trigger_time();
		[[nodiscard]] bool ready() const;
		[[nodiscard]] long millis_until_ready() const;
		void stop();
		void start();

		[[nodiscard]] int id() const { return m_id; }
		[[nodiscard]] int delay() const { return m_delay; }
		[[nodiscard]] std::function<void()> call() const { return m_call; }
		[[nodiscard]] Duck::Time trigger_time() const { return m_trigger_time; }
		[[nodiscard]] bool is_interval() const { return m_interval; }
		[[nodiscard]] bool enabled() const { return m_enabled; }

	private:
		bool m_enabled = true;
		int m_id = 0;
		int m_delay = -1;
		std::function<void()> m_call = nullptr;
		Duck::Time m_trigger_time = {0, 0};
		bool m_interval = false;

	};
}