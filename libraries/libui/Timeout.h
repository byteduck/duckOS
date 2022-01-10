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
	class Timeout {
	public:
		Timeout() = default;

		Timeout(int delay, std::function<void()> call, bool is_interval): delay(delay), call(std::move(call)), is_interval(is_interval) {
			calculate_trigger_time();
		}

		void calculate_trigger_time() {
			trigger_time = Duck::Time::now() + Duck::Time::millis(delay);
		}

		[[nodiscard]] bool ready() const {
			return millis_until_ready() <= 0;
		}

		[[nodiscard]] long millis_until_ready() const {
			return (trigger_time - Duck::Time::now()).millis();
		}

		int delay = -1;
		std::function<void()> call = nullptr;
		Duck::Time trigger_time = {0, 0};
		bool is_interval = false;
	};
}