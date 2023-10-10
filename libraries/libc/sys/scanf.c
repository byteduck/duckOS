/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "scanf.h"
#include "printf.h"

#define EXPECT(ch) if((ch) != *buffer++) return num_stored;
#define CONSUME_OPTIONAL(ch) if((ch) == *buffer) buffer++;
#define CONSUME_SIGN \
	if(*buffer == '+') { \
		buffer++; \
	} else if(*buffer == '-') { \
		buffer++; \
		sign = -1; \
	}

void store_num(void* dest, long long num, int size) {
	switch(size) {
		case 1:
			*(int8_t*)dest = num;
			break;
		case 2:
			*(int16_t*)dest = num;
			break;
		case 4:
			*(int32_t*)dest = num;
			break;
		case 8:
			*(int64_t*)dest = num;
			break;
	}
}

int common_scanf(char* buffer, const char* format, va_list arg) {
	int num_stored = 0;
	char* obuf = buffer;
	while(*format && *buffer) {
		if(isspace(*format)) {
			// Consume all whitespace
			while(isspace(*buffer))
				buffer++;
			format++;
			continue;
		} else if(*format != '%') {
			// Other characters
			EXPECT(*format++);
			continue;
		}

		format++;
		uint8_t size = sizeof(int);
		unsigned int arg_width = 0;

		// Interpret the argument width
		if(*format == '*') {
			arg_width = (char) va_arg(arg, int);
			format++;
		} else {
			INTERPRET_NUMBER(format, arg_width);
		}

		//Interpret number size
		if(*format == 'z') {
			size = sizeof(size_t);
			format++;
		} else if(*format == 'l') {
			format++;
			size = sizeof(long int);
			if(*format == 'l') {
				format++;
				size = sizeof(long long int);
			}
		} else if(*format == 'h') {
			format++;
			size = sizeof(uint16_t);
			if(*format == 'h') {
				format++;
				size = sizeof(uint8_t);
			}
		} else if(*format == 'j') {
			format++;
			size = sizeof(intmax_t);
		} else if(*format == 't') {
			format++;
			size = sizeof(ptrdiff_t);
		} else if(*format == 'L') {
			format++;
			size = sizeof(long double);
		} else if(*format == 'h') {
			format++;
			size = sizeof(short);
			if(*format == 'h') {
				format++;
				size = sizeof(char);
			}
		}

		long long sign = 1;
		switch(*format++) {
			case 'c': {
				//Character
				num_stored++;
				char* out = (char*) va_arg(arg, size_t);
				if(!arg_width)
					arg_width = 1;
				while(arg_width--)
					*out++ = *buffer++;
				break;
			}
			case 'p': //Pointer address
				if(!arg_width)
					arg_width = 8;
				goto hex;
			case 'X':
			case 'x': //Hex
				CONSUME_SIGN
				hex: {
					// Consume 0x
					CONSUME_OPTIONAL('0');
					CONSUME_OPTIONAL('x') else CONSUME_OPTIONAL('X');
					// Read number
					num_stored++;
					store_num((void*) va_arg(arg, size_t), sign * strtoull(buffer, &buffer, 16), size);
					break;
				}
			case 'i':
				CONSUME_SIGN
				// Figure out base
				if(*format == '0') {
					format++;
					if(*format == 'x') {
						format++;
						goto hex;
					} else {
						goto oct;
					}
				}
				goto dec;
			case 'd': //Dec
				CONSUME_SIGN
				dec: {
					num_stored++;
					store_num(va_arg(arg, void*), sign * strtoll(buffer, &buffer, 10), size);
					break;
				}
			case 'u': //Unsigned dec
				CONSUME_SIGN
				{
					num_stored++;
					store_num(va_arg(arg, void*), sign * strtoull(buffer, &buffer, 10), size);
					break;
				}
			case 'o': //Oct
				CONSUME_SIGN
				oct: {
					num_stored++;
					store_num(va_arg(arg, void*), sign * strtoull(buffer, &buffer, 8), size);
					break;
				}
			case 'F':
			case 'f':
			case 'e':
			case 'g':
			case 'a': // Float
			{
				num_stored++;
				long double val = strtold(buffer, &buffer);
				switch(arg_width) {
					case 0:
						*va_arg(arg, float*) = (float) val;
						break;
					case sizeof(long int):
						*va_arg(arg, double*) = (double) val;
						break;
					case sizeof(long double):
						*va_arg(arg, long double*) = val;
						break;
					default:
						break;
				}
				break;
			}
			case 's': //String
			{
				if(!arg_width)
					arg_width = -1;
				num_stored++;
				char* str = (char*) va_arg(arg, char*);
				while(!isspace(*buffer) && *buffer && arg_width--)
					*str++ = *buffer++;
				*str = '\0';
				break;
			}
			case '%':
				EXPECT('%');
				break;
			case '[': {
				bool negate = false;
				if(*format == '^') {
					negate = true;
					format++;
				}
				const char* chset = format;
				while(*format != ']')
					format++;
				for(const char* cmp = chset; cmp != format && *buffer; cmp++) {
					if(*buffer == *cmp) {
						if(negate) {
							break;
						} else {
							buffer++;
							cmp = chset - 1;
						}
					}
					if(negate && cmp + 1 == format) {
						buffer++;
						cmp = chset - 1;
					}
				}
				format++;
				break;
			}
			case 'n':
				num_stored++;
				*va_arg(arg, int*) = buffer - obuf;
				break;
			default:
				break;
		}
	}

	return num_stored;
}