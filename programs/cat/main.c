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

//A program that writes the contents of a file to stdout.

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char** argv) {
	int f;

	if(!isatty(STDIN_FILENO)) {
		f = STDIN_FILENO;
	} else if(argc < 2) {
		printf("cat: Missing file operand\nUsage: cat FILE\n");
		return 1;
	} else {
		f = open(argv[1], O_RDONLY);
	}

	if(f != -1) {
		char buf[512];
		int nread;
		while((nread = read(f, buf, 512)) > 0) {
			if(write(STDOUT_FILENO, buf, nread) < 0) {
				perror("cat");
				return errno;
			}
		}
		if(nread < 0 && errno){
			perror("cat");
			return errno;
		}
	} else {
		perror("cat");
		return errno;
	}
	return 0;
}
