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

#include "FormatStream.h"
using namespace Duck;

FormatParams::FormatParams(InputStream& stream) {
	enum {
		REGULAR,
		PRECISION
	} state = REGULAR;

	std::string precision_string;

	while(!stream.eof()) {
		char ch = stream.getchar();
		switch(state) {
			case REGULAR: {
				regstate:
				switch(ch) {
					case '{':
						literal_brace = true;
						return;
					case 'c':
						number_format = CHARACTER;
						break;
					case 'x':
						number_format = HEX;
						break;
					case 'X':
						number_format = HEX_UPPER;
						break;
					case 'd':
						number_format = DECIMAL;
						break;
					case '#':
						alternate_form = true;
						break;
					case '.':
						state = PRECISION;
						precision_string = "";
						break;
					case '}':
						return;
					default:
						break;
				}
				break;
			}

			case PRECISION: {
				if(ch >= '0' && ch <= '9') {
					precision_string += ch;
				} else {
					precision = atoi(precision_string.c_str());
					state = REGULAR;
					goto regstate;
				}
			}
		}

	}
}

FormatStream::FormatStream(OutputStream& stream, std::string format):
	m_stream(stream), m_format(std::move(format)), m_fmt_stream(m_format) {}

FormatStream::~FormatStream() {
	//Flush the rest of the format string
	while(!m_fmt_stream.eof()) {
		output_until_format();
		if(read_format_params().literal_brace)
			m_stream << "{";
	}
}

void FormatStream::output_until_format() {
	m_fmt_stream.set_delimeter('{');
	std::string buf;
	m_fmt_stream >> buf;
	m_stream << buf;
}

FormatParams FormatStream::read_format_params() {
	return FormatParams(m_fmt_stream);
}

void FormatStream::write_hex(FormatParams params, unsigned long long num) {
	//Figure out the width of the resulting string
	size_t num_chars = 0;
	unsigned int numchar_calc = num;
	do {
		num_chars++;
		numchar_calc /= 0x10;
	} while(numchar_calc);

	//If we have to print the prefix, do so
	if(params.alternate_form)
		m_stream << "0x";

	//Output the hex
	const char* hex = params.number_format == FormatParams::HEX_UPPER ? "0123456789ABCDEF" : "0123456789abcdef";
	for(size_t i = 1; i <= num_chars; i++)
		m_stream.putchar(hex[(num >> ((num_chars - i) * 4)) & 0xFu]);
}

void FormatStream::write_arg(FormatParams params, long long arg) {
	switch(params.number_format) {
		case FormatParams::CHARACTER:
			m_stream.putchar((char) arg);
			break;
		case FormatParams::HEX:
		case FormatParams::HEX_UPPER:
			write_hex(params, arg);
			break;
		case FormatParams::DECIMAL:
			m_stream << arg;
			break;
	}
}

void FormatStream::write_arg(FormatParams params, unsigned long long int arg) {
	switch(params.number_format) {
		case FormatParams::CHARACTER:
			m_stream.putchar((char) arg);
			break;
		case FormatParams::HEX:
		case FormatParams::HEX_UPPER:
			write_hex(params, arg);
			break;
		case FormatParams::DECIMAL:
			m_stream << arg;
			break;
	}
}

void FormatStream::write_arg(FormatParams params, long double arg) {
	//Write the part before the decimal
	m_stream << ((long) arg);

	if(params.precision != 0)
		m_stream.putchar('.');
	else
		return;

	//Print decimal places
	int num_places = (params.precision > -1 && params.precision < 8) ? params.precision : 8;
	for (int d = 0; d < num_places; d++) {
		if ((int) (arg * 100000.0) % 100000 == 0 && d != 0)
			break;
		arg *= 10.0;
		m_stream.putchar("0123456789"[((int) arg) % 10]);
	}
}

namespace Duck {
	FormatStream operator%(OutputStream& stream, const char* fmt) {
		return {stream, fmt};
	}
}