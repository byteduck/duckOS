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

#include <cstdio>
#include <cerrno>
#include "ld.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <malloc.h>

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

	if(executable.load(false) < 0)
		return errno;

	fprintf(stderr, "Not implemented.\n");

	return -1;
}

int sys_internal_alloc(void* addr, size_t size) {
	//TODO: Better way of doing this
	int ret = 0;
	asm volatile("int $0x80" :: "a"(59), "b"(addr), "c"(size));
	asm volatile("mov %%eax, %0" : "=r"(ret));
	if(ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

int Object::load(bool position_independent) {
	//Read the header
	if(read_header() < 0) {
		fprintf(stderr, "Could not read object file header: %s\n", strerror(errno));
		return -1;
	}

	//Read the program headers
	if(read_pheaders() < 0) {
		fprintf(stderr, "Could not read object segment headers: %s\n", strerror(errno));
		return -1;
	}

	//Calculate the object size
	if(calculate_memsz() < 0) {
		fprintf(stderr, "Could not calculate object size.\n");
		errno = ENOEXEC;
		return -1;
	}

	//Allocate memory to hold the object
	if(position_independent) {
		memloc = (size_t) malloc(memsz);
		if (!memloc) {
			fprintf(stderr, "Could not allocate memory for object.\n");
			errno = ENOMEM;
			return -1;
		}
	} else {
		memloc = 0;
		if(sys_internal_alloc((void*) calculated_base, memsz) < 0) {
			fprintf(stderr, "Could not allocate memory for object: %s\n", strerror(errno));
			return -1;
		}
	}

	//Load the object
	if(load_sections() < 0) {
		fprintf(stderr, "Could not load object into memory: %s\n", strerror(errno));
		return -1;
	}

	if(read_dynamic_table() < 0) {
		fprintf(stderr, "Could not read object dynamic table: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}


int Object::read_header() {
	lseek(fd, 0, SEEK_SET);
	if(read(fd, &header, sizeof(elf32_header)) < 0) {
		return -1;
	}

	if(header.magic != MAGIC) {
		errno = ENOEXEC;
		return -1;
	}

	return 0;
}

int Object::calculate_memsz() {
	size_t base = -1;
	size_t brk = 0;
	for(auto & pheader : pheaders) {
		switch(pheader.p_type) {
			case PT_LOAD:
				if(pheader.p_vaddr < base)
					base = pheader.p_vaddr;
				if(pheader.p_vaddr + pheader.p_memsz > brk)
					brk = pheader.p_vaddr + pheader.p_memsz;
				break;
			default:
				break;
		}
	}

	if(base == -1) return -ENOEXEC;
	memsz = brk - base;
	calculated_base = base;
	return 0;
}

int Object::read_pheaders() {
	uint32_t pheader_loc = header.program_header_table_position;
	uint32_t pheader_size = header.program_header_table_entry_size;
	uint32_t num_pheaders = header.program_header_table_entries;

	//Seek to the pheader_loc
	if(lseek(fd, pheader_loc, SEEK_SET) < -1)
		return -1;

	//Create the segment header vector and read the headers into it
	pheaders.resize(num_pheaders);
	if(read(fd, (uint8_t*) pheaders.data(), pheader_size * num_pheaders) < 0)
		return -1;

	return 0;
}

int Object::read_dynamic_table() {
	bool did_read = false;
	for(auto & pheader : pheaders) {
		if(pheader.p_type == PT_DYNAMIC) {
			//Read the dynamic table
			did_read = true;
			std::vector<elf32_dynamic> dynamic_table(pheader.p_filesz / sizeof(elf32_dynamic));
			if(lseek(fd, pheader.p_offset, SEEK_SET) < 0)
				return -1;
			if(read(fd, dynamic_table.data(), pheader.p_filesz) < 0)
				return -1;

			//Iterate over dynamic table entries
			for(auto & dynamic : dynamic_table) {
				if(dynamic.d_tag == DT_NULL) break;
				switch(dynamic.d_tag) {
					case DT_HASH:
						hash = (uint32_t*) (memloc + dynamic.d_val);
						//Size of symbol table should be the same as the number of entries in the symbol hash table
						symbol_table_size = hash[1];
						break;

					case DT_STRTAB:
						string_table = (char*) (memloc + dynamic.d_val);
						break;

					case DT_SYMTAB:
						symbol_table = (elf32_sym*) (memloc + dynamic.d_val);
						break;

					case DT_STRSZ:
						string_table_size = dynamic.d_val;
						break;

					case DT_INIT:
						init_func = (void(*)()) (memloc + dynamic.d_val);
						break;

					case DT_INIT_ARRAY:
						init_array = (void(**)()) (memloc + dynamic.d_val);
						break;

					case DT_INIT_ARRAYSZ:
						init_array_size = dynamic.d_val;
						break;
				}
			}

			//Now that the string table is loaded, we can iterate again and find the required libraries
			libraries.resize(0);
			for(auto & dynamic : dynamic_table) {
				if(dynamic.d_tag == DT_NEEDED) {
					libraries.push_back(string_table + dynamic.d_val);
				}
			}
		}
	}

	if(!did_read)
		return -ENOENT;

	return 0;
}

int Object::load_sections() {
	for(auto & pheader : pheaders) {
			switch(pheader.p_type) {
			case PT_LOAD: {
				//Load the section into memory
				if(lseek(fd, pheader.p_offset, SEEK_SET) < 0)
					return -1;
				if(read(fd, (void*) (memloc + pheader.p_vaddr), pheader.p_filesz) < 0)
					return -1;

				//Zero out the remaining bytes
				size_t bytes_left = pheader.p_memsz - pheader.p_filesz;
				if(bytes_left)
					memset((void*) (memloc + pheader.p_vaddr + pheader.p_filesz), 0, bytes_left);
			}
		}
	}

	return 0;
}
