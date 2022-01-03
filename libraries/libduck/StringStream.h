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

#include <string>
#include <utility>
#include "Stream.h"

namespace Duck {
	class StringInputStream: public InputStream {
	public:
		explicit StringInputStream(std::string string): m_string(std::move(string)) {}

		//InputStream
		size_t read(void* buffer, size_t n) override;
		Result seek(long offset, Whence whence) override;
		[[nodiscard]] bool eof() const override { return m_eof; }

	private:
		std::string m_string;
		size_t m_offset = 0;
		bool m_eof = false;
	};

	class StringOutputStream: public OutputStream {
	public:
		const std::string& string() { return m_string; }

		//OutputStream
		size_t write(const void* buffer, size_t n) override;
		Result seek(long seek, Whence whence) override;

	private:
		std::string m_string;
		size_t m_offset = 0;
	};

}


