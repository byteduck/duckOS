#include <kernel/tasking/elf.h>
#include <common/defines.h>
#include <kernel/filesystem/VFS.h>
#include <kernel/memory/paging.h>
#include "tasking.h"

bool ELF::is_valid_elf_header(elf32_header* header) {
	return header->magic == ELF_MAGIC;
}

bool ELF::can_execute(elf32_header *header) {
	if(!is_valid_elf_header(header)) return false;
	if(header->bits != ELF32) return false;
	if(header->instruction_set != ELF_X86) return false;
	if(header->elf_version != 0x1) return false;
	if(header->header_version != 0x1) return false;
	if(header->type != ELF_TYPE_EXECUTABLE) return false;
	return header->endianness == ELF_LITTLE_ENDIAN;
}

void ELF::load_and_execute(DC::string file) {
	/*//FIXME: Do not use. Doesn't deallocate memory and I only wrote this to test loading ELFs.
	auto fd_or_error = VFS::inst().open(file, O_RDONLY, 0, VFS::inst().root_ref());
	if(fd_or_error.is_error()) {
		printf("Failed to open.\n");
		return;
	}
	auto fd = fd_or_error.value();
	auto* header = new elf32_header;
	fd->read((uint8_t*)header, sizeof(elf32_header));
	if(!can_execute(header)) {
		printf("Cannot execute.\n");
		delete header;
		return;
	}

	uint32_t pheader_loc = header->program_header_table_position;
	uint32_t pheader_size = header->program_header_table_entry_size;
	uint32_t num_pheaders = header->program_header_table_entries;

	fd->seek(pheader_loc, SEEK_SET);
	auto* program_headers = new ELF::elf32_segment_header[num_pheaders];
	fd->read((uint8_t*)program_headers, pheader_size * num_pheaders);

	for(auto i = 0; i < num_pheaders; i++) {
		auto pheader = &program_headers[i];
		if(pheader->p_type == ELF_PT_LOAD) {
			size_t loadloc_pagealigned = (pheader->p_vaddr/PAGE_SIZE) * PAGE_SIZE;
			if(!allocate_pages(loadloc_pagealigned, pheader->p_memsz)) {
				printf("Failed to allocate page at 0x%x\n.", loadloc_pagealigned);
				delete[] program_headers;
				delete header;
				return;
			}
			printf("Allocated page at 0x%x (0x%x bytes)\n", loadloc_pagealigned, pheader->p_memsz);
			fd->seek(pheader->p_offset, SEEK_SET);
			auto* buf = new uint8_t[pheader->p_filesz];
			fd->read(buf, pheader->p_filesz);
			auto* vmem = (uint8_t*)pheader->p_vaddr;
			memcpy(vmem, buf, pheader->p_filesz);
			delete[] buf;
		}
	}

	auto* proc = createProcess(file.c_str(), header->program_entry_position);
	addProcess(proc);
	while(proc->state == PROCESS_ALIVE);


	delete[] program_headers;
	delete header;*/
}