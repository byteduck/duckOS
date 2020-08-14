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

// A program that changes the mode of a file.

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

int main(int argc, char** argv) {
	if(argc < 3) {
		printf("chmod: Missing operands\nUsage: chmod MODE FILE\n");
		return 1;
	}

	char* endptr;
	long mode = strtol(argv[1], &endptr, 0);
	if (endptr == argv[1]) {
		printf("chmod: Invalid mode\n");
		return EINVAL;
	}
	if ((mode == LONG_MAX || mode == LONG_MIN) && errno == ERANGE) {
		printf("chmod: Invalid mode\n");
		return ERANGE;
	}

	if(chmod(argv[2], mode) < 0) {
		perror("chmod");
		return errno;
	}

	return 0;
}
