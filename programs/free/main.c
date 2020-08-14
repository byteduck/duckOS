#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

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

// A program that shows memory info

#define GiB 1073742000
#define MiB 1048576
#define KiB 1024

double get_human_size(unsigned long bytes) {
	if(bytes > GiB) {
		return bytes / (double) GiB;
	} else if(bytes > MiB) {
		return bytes / (double) MiB;
	} else if(bytes > KiB) {
		return bytes / (double) KiB;
	} else return bytes;
}

char* get_human_suffix(unsigned long bytes) {
	if(bytes > GiB) {
		return "GiB";
	} else if(bytes > MiB) {
		return "MiB";
	} else if(bytes > KiB) {
		return "KiB";
	} else return "bytes";
}

int main(int argc, char** argv, char** envp) {
	int fd = open("/proc/meminfo", O_RDONLY);

	if(fd < 0) {
		perror("free");
		return errno;
	}

	char* buf = malloc(1024);
	if(read(fd, buf, 1024) < 0) {
		perror("free");
		return errno;
	}

	unsigned long usable = 0;
	unsigned long used = 0;

	char* key = strtok(buf, ":");
	do {
		char* valuestr = strtok(NULL, "\n");
		long value = strtol(valuestr, NULL, 0);

		if(!strcmp(key, "Usable")) usable = value;
		else if(!strcmp(key, "Used")) used = value;
	} while((key = strtok(NULL, ":")));

	unsigned long available = usable - used;
	double used_mem_frac = (double)((unsigned long)(used / 1024)) / (double)((unsigned long)(usable / 1024));

	if(argc >= 2 && !strcmp(argv[1], "-h")) {
		printf("Total: %.2f %s\n", get_human_size(usable), get_human_suffix(usable));
		printf("Used: %.2f %s (%.2f%%)\n", get_human_size(used), get_human_suffix(used), used_mem_frac * 100.0);
		printf("Available: %.2f %s (%.2f%%)\n", get_human_size(available), get_human_suffix(available), (1 - used_mem_frac) * 100.0);
	} else {
		printf("Total: %lu\n", usable);
		printf("Used: %lu (%.2f%%)\n", used, used_mem_frac * 100.0);
		printf("Available: %lu (%.2f%%)\n", available, (1 - used_mem_frac) * 100.0);
	}

	return 0;
}