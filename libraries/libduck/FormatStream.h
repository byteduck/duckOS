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

#include "Stream.h"
#include "StringStream.h"

#pragma once

namespace Duck {
	class FormatParams {
	public:
		explicit FormatParams(InputStream& stream);

		enum {
			CHARACTER,
			DECIMAL,
			HEX,
			HEX_UPPER
		} number_format = DECIMAL;
		int precision = -1;
		bool alternate_form = false;
		bool literal_brace = false;
	};

	class FormatStream {
	public:
		FormatStream(OutputStream& stream, std::string format);
		~FormatStream();

		template<typename T>
		FormatStream& operator%(const T& arg) {
			feed(arg);
			return *this;
		}

	private:
		void output_until_format();
		FormatParams read_format_params();

		template<typename T>
		void feed(const T& arg) {
			feed_start:
			output_until_format();
			if(m_fmt_stream.eof())
				return; //If we reached EOF, there's no argument to print
			auto fmt_params = read_format_params();

			//If the format parameter is '{', that means just print '{' and look for the next argument
			if(fmt_params.literal_brace) {
				m_stream << "{";
				goto feed_start;
			}

			//Write the argument
			write_arg(fmt_params, arg);
		}

		//Base formatters
		void write_hex(FormatParams params, unsigned long long arg);

		//Integers
		void write_arg(FormatParams params, long long arg);
		void write_arg (FormatParams params, long arg) { write_arg(params, (long long) arg); }
		void write_arg (FormatParams params, int arg) { write_arg(params, (long long) arg); }
		void write_arg (FormatParams params, char arg) { write_arg(params, (long long) arg); }
		void write_arg(FormatParams params, unsigned long long arg);
		void write_arg(FormatParams params, unsigned long arg) { write_arg(params, (unsigned long long) arg); }
		void write_arg(FormatParams params, unsigned int arg) { write_arg(params, (unsigned long long) arg); }

		//Floats and Doubles
		void write_arg(FormatParams params, long double arg);
		void write_arg(FormatParams params, double arg) { write_arg(params, (long double) arg); }
		void write_arg(FormatParams params, float arg) { write_arg(params, (long double) arg); }

		//All other types of args
		template<typename T>
		void write_arg(FormatParams params, const T& arg) {
			m_stream << arg;
		}

		OutputStream& m_stream;
		std::string m_format;
		StringInputStream m_fmt_stream;
	};

	FormatStream operator%(OutputStream& stream, const char* fmt);

	template<typename... T>
	void sprint(OutputStream& stream, const char* fmt, T... args) {
		(stream % fmt % ... % args);
	}

	template<typename... T>
	void sprintln(OutputStream& stream, const char* fmt, T... args) {
		sprint(stream, fmt, args...);
		stream << "\n";
	}

	template<typename... T>
	void print(const char* fmt, T... args) {
		sprint(Stream::std_out, fmt, args...);
	}

	template<typename... T>
	void println(const char* fmt, T... args) {
		sprintln(Stream::std_out, fmt, args...);
	}

	template<typename... T>
	void printerr(const char* fmt, T... args) {
		sprint(Stream::std_err, fmt, args...);
	}

	template<typename... T>
	void printerrln(const char* fmt, T... args) {
		sprintln(Stream::std_err, fmt, args...);
	}
}

