#include <kernel/tasking/elf.h>
#include <common/defines.h>

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