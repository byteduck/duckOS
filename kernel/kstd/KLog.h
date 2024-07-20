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

#include "../tasking/Mutex.h"
#include "../Result.hpp"
#include "../api/strerror.h"

extern Mutex printf_lock;

namespace KLog {
	void print_header(const char* component, const char* color, const char* type);

	inline void print_arg(char arg) {
		putch(arg);
	}

	struct FormatRules {
		bool hex = false;
		bool prefix = false;
		bool upper = false;
	};

#define __printarg_num(type) \
	inline void print_arg(type arg, FormatRules rules) { \
        if (rules.hex) \
			printf(rules.prefix ? rules.upper ? "0x%X" : "0x%x" : rules.upper ? "%X" : "%x", (long) arg); \
    	else \
    		printf("%d", (long) arg); \
	}

	__printarg_num(unsigned char);
	__printarg_num(short);
	__printarg_num(unsigned short);
	__printarg_num(int);
	__printarg_num(unsigned int);
	__printarg_num(long);
	__printarg_num(unsigned long);
	__printarg_num(long long);
	__printarg_num(unsigned long long);
	__printarg_num(const void*);

	inline void print_arg(const kstd::string& str, FormatRules rules) {
		print(str.c_str());
	}

	inline void print_arg(const char* str, FormatRules rules) {
		print(str);
	}

	template<typename Arg>
	const char* format_doprint_arg(const char* fmt, const Arg& arg) {
		enum { Printing, FmtBegin, FmtArgs } state = Printing;
		FormatRules rules;
		while (*fmt) {
			if (state == Printing) {
				if (*fmt == '{')
					state = FmtBegin;
				else
					putch(*fmt);
				fmt++;
			} else if (state == FmtBegin) {
				if (*fmt == '{') {
					state = Printing;
					putch(*fmt);
					fmt++;
				} else {
					state = FmtArgs;
				}
			} else if (state == FmtArgs) {
				if (*fmt == '}') {
					print_arg(arg, rules);
					fmt++;
					break;
				} else if (*fmt == 'x') {
					rules.hex = true;
				} else if (*fmt == 'X' ) {
					rules.upper = true;
				} else if(*fmt == '#') {
					rules.prefix = true;
				}
				fmt++;
			}
		}
		return fmt;
	}

	inline void format_doprint(const char* fmt) {
		print(fmt);
	}

	template<typename Arg>
	void format_doprint(const char* fmt, const Arg& arg) {
		fmt = format_doprint_arg(fmt, arg);
		print(fmt);
	}

	template<typename Arg, typename... Rest>
	void format_doprint(const char* fmt, const Arg& arg, const Rest... rest) {
		fmt = format_doprint_arg(fmt, arg);
		format_doprint(fmt, rest...);
	}

	template<typename... ArgTs>
	void dbg(const char* component, const char* fmt, const ArgTs&... args) {
		LOCK(printf_lock);
		print_header(component, "90", "DEBUG");
		format_doprint(fmt, args...);
		putch('\n');
	}

	template<typename... ArgTs>
	void info(const char* component, const char* fmt, const ArgTs&... args) {
		LOCK(printf_lock);
		print_header(component, "94", "INFO");
		format_doprint(fmt, args...);
		putch('\n');
	}

	template<typename... ArgTs>
	void success(const char* component, const char* fmt, const ArgTs&... args) {
		LOCK(printf_lock);
		print_header(component, "92", "SUCCESS");
		format_doprint(fmt, args...);
		putch('\n');
	}

	template<typename... ArgTs>
	void warn(const char* component, const char* fmt, const ArgTs&... args) {
		LOCK(printf_lock);
		print_header(component, "93", "WARN");
		format_doprint(fmt, args...);
		putch('\n');
	}

	template<typename... ArgTs>
	void err(const char* component, const char* fmt, const ArgTs&... args) {
		LOCK(printf_lock);
		print_header(component, "91", "ERROR");
		format_doprint(fmt, args...);
		putch('\n');
	}

	template<typename... ArgTs>
	void crit(const char* component, const char* fmt, const ArgTs&... args) {
		LOCK(printf_lock);
		print_header(component, "97;41", "CRITICAL");
		format_doprint(fmt, args...);
		putch('\n');
	}

	inline void print_arg(const Result& result, FormatRules rules) {
		printf("%s (%d)", strerror(result.code()), result.code());
	}

	template<typename T>
	inline void print_arg(const ResultRet<T>& result, FormatRules rules) {
		if (result.is_error())
			print_arg(result.result(), rules);
		else
			print_arg(result.value(), rules);
	}

#define KLOG_CONDITIONAL(method) \
	template<bool C, typename... ArgTs> \
	constexpr void method##_if(const char* component, const char* fmt, ArgTs... args) { \
		if constexpr(C) \
			method(component, fmt, args...); \
	}

	KLOG_CONDITIONAL(dbg);
	KLOG_CONDITIONAL(info);
	KLOG_CONDITIONAL(success);
	KLOG_CONDITIONAL(warn);
	KLOG_CONDITIONAL(err);
	KLOG_CONDITIONAL(crit);
}
