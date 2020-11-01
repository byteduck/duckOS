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
    
    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <sys/printf.h>
#include <stdio.h>

void hex_str(unsigned int val, char** buf, unsigned int width, size_t n, size_t* len, bool upper, char pad) {
	//Figure out the width of the resulting string
	size_t num_chars = 0;
	unsigned int val2 = val;
	do {
		num_chars ++;
		val2 /= 0x10;
	} while(val2);

	//Pad
	for(size_t i = num_chars; i < width && *len < n; i++) {
		**buf = pad;
		(*buf)++;
		(*len)++;
	}

	if(*len - n < num_chars)
		num_chars = *len - n;

	//Print the hex
	char* hex = upper ? "0123456789ABCDEF" : "0123456789abcdef";
	for(size_t i = 1; i <= num_chars; i++) {
		**buf = hex[(val >> ((num_chars - i) * 4)) & 0xFu];
		(*buf)++;
	}

	(*len) += num_chars;
}

void dec_str(unsigned int val, char** buf, unsigned int width, size_t n, size_t* len, char pad) {
	//Figure out the width of the resulting string
	size_t num_chars = 0;
	unsigned int val2 = val;
	do {
		num_chars++;
		val2 /= 10;
	} while(val2);

	//Pad
	for(size_t i = num_chars; i < width && *len < n; i++) {
		**buf = pad;
		(*buf)++;
		(*len)++;
	}

	if(*len - n < num_chars)
		num_chars = *len - n;

	(*buf) += num_chars - 1;

	//Print the decimal
	for(size_t i = 0; i < num_chars; i++) {
		**buf = "0123456789"[val % 10];
		val /= 10;
		(*buf)--;
	}

	(*len) += num_chars;
	(*buf) += num_chars + 1;
}

int common_printf(char* s, size_t n, const char* format, va_list arg) {
	char* buf = s;
	size_t len = 0;
	for(const char* p = format; *p && len < n; p++) {
		int precision = -1;

		//If it's not a percent, add it to the buffer
		if(*p != '%') {
			*buf++ = *p;
			len++;
			continue;
		}

		p++;
		bool zero_pad = false;
		bool force_sign = false;
		bool left_justify = false;
		bool alt_prefix = false;
		bool space_no_sign = false;
		uint8_t size = 0;
		unsigned int arg_width = 0;

		//Interpret arguments to the format
		while(1) {
			char ch = *p;
			if(ch == '-') {
				left_justify = true;
				p++;
			} else if(ch == '+') {
				force_sign = true;
				p++;
			} else if(ch == '0') {
				zero_pad = true;
				p++;
			} else if(ch == '*') {
				arg_width = (char) va_arg(arg, int);
				p++;
			} else if(ch == '#') {
				alt_prefix = true;
				p++;
			} else if(ch == ' ') {
				space_no_sign = true;
				p++;
			} else break;
		}

		//Interpret the argument width
		INTERPRET_NUMBER(p, arg_width);

		//Interpret argument precision
		if(*p == '.') {
			p++;
			precision = 0;
			if(*p == '*') {
				//Precision is given in argument list
				precision = (int) va_arg(arg, int);
				p++;
			} else {
				//Precision is given in string
				INTERPRET_NUMBER(p, precision);
			}
		}

		//Interpret number size
		if(*p == 'z') {
			size = 1;
			p++;
		} else if(*p == 'l') {
			p++;
			size = 1;
			if(*p == 'l') {
				p++;
				size = 2;
			}
		}

		bool uppercase = false;
		switch(*p) {
			case 'c': //Character
				*buf++ = (char) va_arg(arg, int);
				break;

			case 'p': //Pointer address
				if(!arg_width)
					arg_width = 8;
				zero_pad = true;
				goto hex;
			case 'X':
				uppercase = true;
			case 'x': //Hex
			hex:
				if(alt_prefix) {
					*buf++ = '0';
					*buf++ = 'x';
					len += 2;
				}

				//Precision can also be used for zero padding with ints / hex
				if(precision != -1) {
					arg_width = precision;
					zero_pad = true;
				}

				if(size == 2) { //long long
					unsigned long long val = (unsigned long long) va_arg(arg, unsigned long long);
					if(val > 0xFFFFFFFF)
						hex_str(val >> 32, &buf, arg_width > 8 ? (arg_width - 8) : 0, n, &len, uppercase, zero_pad ? '0' : ' ');
					hex_str(val & 0xFFFFFFFF, &buf, arg_width > 8 ? 8 : arg_width, n, &len, uppercase, zero_pad ? '0' : ' ');
				} else { //unsigned long or smaller
					unsigned long val = va_arg(arg, unsigned long);
					hex_str(val, &buf, arg_width, n, &len, uppercase, zero_pad ? '0' : ' ');
				}
				break;

			case 'i':
			case 'd': //Dec
			{
				long long val;
				if(size == 2)
					val = (long long) va_arg(arg, long long);
				else
				val = (long long) va_arg(arg, long);

				//Print sign if necessary
				if(val < 0) {
					*buf++ = '-';
					val = -val;
					len++;
				} else if(force_sign) {
					*buf++ = '+';
					len++;
				} else if(space_no_sign) {
					*buf++ = ' ';
					len++;
				}

				//Precision can also be used for zero padding with ints / hex
				if(precision != -1) {
					arg_width = precision;
					zero_pad = true;
				}

				dec_str(val, &buf, arg_width, n, &len, zero_pad ? '0' : ' ');
				break;
			}

			case 'u': //Unsigned dec
			{
				unsigned long long val;
				if(size == 2)
					val = (unsigned long long) va_arg(arg, unsigned long long);
				else
					val = (unsigned long long) va_arg(arg, unsigned long);

				//Print sign if necessary
				if(force_sign) {
					*buf++ = '+';
					len++;
				} else if(space_no_sign) {
					*buf++ = ' ';
					len++;
				}

				//Precision can also be used for zero padding with ints / hex
				if(precision != -1) {
					arg_width = precision;
					zero_pad = true;
				}

				dec_str(val, &buf, arg_width, n, &len, zero_pad ? '0' : ' ');
				break;
			}

			case 'F':
				uppercase = true;
			case 'f': //Double
			{
				double val = (double) va_arg(arg, double);

				//Print sign if necessary
				if(val < 0) {
					*buf++ = '-';
					val = -val;
					len++;
				} else if(force_sign) {
					*buf++ = '+';
					len++;
				} else if(space_no_sign) {
					*buf++ = ' ';
					len++;
				}

				//Print before decimal
				dec_str((int) val, &buf, arg_width, n, &len, zero_pad ? '0' : ' ');

				//Print decimal
				if(len < n) {
					*buf++ = '.';
					len++;
				}

				//Print decimal places
				int num_places = (precision > -1 && precision < 8) ? precision : 8;
				for (int d = 0; d < num_places && len < n; d++) {
					if ((int) (val * 100000.0) % 100000 == 0 && d != 0)
						break;
					val *= 10.0;
					*buf++ = "0123456789"[((int) val) % 10];
					len++;
				}

				break;
			}

			case 's': //String
			{
				char* str = (char*) va_arg(arg, char*);
				if(!str) str = "(null)";
				size_t nchr = 0;
				//Precision and arg width are used as string length
				if(precision < 0) {
					while(*str && len < n) {
						*buf++ = *str++;
						nchr++;
						len++;
						if(arg_width && nchr == arg_width) break;
					}
				} else {
					while(*str && precision && len < n) {
						*buf++ = *str++;
						nchr++;
						precision--;
						len++;
						if(arg_width && nchr == arg_width) break;
					}
				}
				break;
			}

			case '%':
				*buf++ = '%';
				len++;
				break;

			default:
				//TODO: e, E, g, G, n
				*buf++ = *p;
				len++;
				break;
		}
	}

	*buf = '\0';
	return len;
}