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

#include <stdio.h>
#include <common/vector.hpp>
#include <errno.h>
#include "ld.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv) {
	if(argc < 2) {
		fprintf(stderr, "No binary specified. Usage: ld-duckos.so BINARY\n");
		return -1;
	}

	Object executable;

	//Open the executable
	executable.fd  = open(argv[1], O_RDONLY);
	if(executable.fd < 0) {
		perror("ld-duckos.so");
		return errno;
	}

	//Read the header
	if(executable.read_header() < 0) {
		fprintf(stderr, "Could not read object file header: %s\n", strerror(errno));
		return errno;
	}

	//Read the segment headers
	if(executable.read_segment_headers() < 0) {
		fprintf(stderr, "Could not read object segment headers: %s\n", strerror(errno));
		return errno;
	}

	fprintf(stderr, "Not implemented.\n");

	return -1;
}

int Object::read_header() {
	lseek(fd, 0, SEEK_SET);
	if(read(fd, &header, sizeof(elf32_header)) < 0) {
		return -1;
	}

	if(header.magic != ELF_MAGIC) {
		errno = ENOEXEC;
		return -1;
	}

	return 0;
}

int Object::read_segment_headers() {
	uint32_t pheader_loc = header.program_header_table_position;
	uint32_t pheader_size = header.program_header_table_entry_size;
	uint32_t num_pheaders = header.program_header_table_entries;

	//Seek to the pheader_loc
	if(lseek(fd, pheader_loc, SEEK_SET) < -1)
		return -1;

	//Create the segment header vector and read the headers into it
	segments.resize(num_pheaders);
	if(read(fd, (uint8_t*) segments.storage(), pheader_size * num_pheaders) < 0)
		return -1;

	return 0;
}
