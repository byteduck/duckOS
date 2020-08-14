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
#include <unistd.h>

long interpret_number(char* string) {
	char* endptr;
	long ret = strtol(string, &endptr, 0);
	if (endptr == string) {
		printf("chown: Invalid id\n");
		exit(EINVAL);
	}
	if ((ret == LONG_MAX || ret == LONG_MIN) && errno == ERANGE) {
		printf("chown: Invalid id\n");
		exit(ERANGE);
	}
	return ret;
}

int main(int argc, char** argv) {
	if(argc < 4) {
		printf("chown: Missing operands\nUsage: chown UID GID FILE\n");
		return 1;
	}

	if(chown(argv[3], interpret_number(argv[1]),  interpret_number(argv[2])) < 0) {
		perror("chown");
		return errno;
	}

	return 0;
}
