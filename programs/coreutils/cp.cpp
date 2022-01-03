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

//A program that copies a file.

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char** argv) {
	if(argc < 3) {
		printf("Missing operands\nUsage: cp FILE NEWFILE\n");
		return 1;
	}

	int from_fd = open(argv[1], O_RDONLY);
	if(from_fd == -1) {
		perror("cp");
		return errno;
	}

	struct stat from_st;
	if(stat(argv[1], &from_st) < 0) {
		perror("cp");
		return errno;
	}

	int to_fd = open(argv[2], O_WRONLY | O_CREAT, from_st.st_mode);
	if(to_fd == -1) {
		perror("cp");
		return errno;
	}

	errno = 0;

	ssize_t nread;
	auto* buf = (char*) malloc(1024);
	while((nread = read(from_fd, buf, 1024))) {
		ssize_t nwrote;
		nwrote = write(to_fd, buf, nread);
		if(nwrote <= 0) {
			if(errno) {
				perror("cp");
				return errno;
			}
			perror("cp");
			return 1;
		}
	}

	close(to_fd);
	close(from_fd);
	free(buf);

	return 0;
}
