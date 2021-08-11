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

#include <cstdio>
#include <cerrno>
#include <cstring>

namespace KLog {
	FILE* klog = nullptr;
	const char* klogid = nullptr;

	void init(const char* identifier) {
		klogid = identifier;
		if(!klog) {
			klog = fopen("/dev/klog", "we");
			setvbuf(klog, nullptr, _IOLBF, 2048);
			if(!klog)
				fprintf(stderr, "Couldn't open kernel log: %s\n", strerror(errno));
		}
	}

	void logf(const char* format, ...) {
		if(!klog) {
			fprintf(stderr, "Attempted to write to kernel log without initializing it first!\n");
			return;
		}

		va_list arg;
		va_start(arg, format);
		fprintf(klog, "[%s] ", klogid);
		vfprintf(klog, format, arg);
		va_end(arg);
	}

	FILE* klog_file() {
		return klog;
	}
}