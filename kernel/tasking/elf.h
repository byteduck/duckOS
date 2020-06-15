#ifndef ELF_H
#define ELF_H

#include <kernel/kstddef.h>
#include <kernel/kstdio.h>
#include <kernel/filesystem/FileDescriptor.h>

#define ELF_MAGIC 0x464C457F //0x7F followed by 'ELF'

#define ELF32 1
#define ELF64 2

#define ELF_LITTLE_ENDIAN 1
#define ELF_BIG_ENDIAN 2

#define ELF_TYPE_RELOCATABLE 1
#define ELF_TYPE_EXECUTABLE 2
#define ELF_TYPE_SHARED 3
#define ELF_TYPE_CORE 4

#define ELF_NO_ARCH 0
#define ELF_X86 3

#define ELF_PT_NULL 0
#define ELF_PT_LOAD 1
#define ELF_PT_DYNAMIC 2
#define ELF_PT_INTERP 3
#define ELF_PT_NOTE 4
#define ELF_PT_SHLIB 5
#define ELF_PT_PHDR 6

#define ELF_PF_X 1u
#define ELF_PF_W 2u
#define ELF_PF_R 4u


namespace ELF {
	typedef struct __attribute__((packed)) elf32_header {
		uint32_t magic;
		uint8_t bits; //1 == 32 bit, 2 == 64 bit
		uint8_t endianness;
		uint8_t header_version;
		uint8_t os_abi;
		uint8_t padding[8];
		uint16_t type;
		uint16_t instruction_set;
		uint32_t elf_version;
		uint32_t program_entry_position;
		uint32_t program_header_table_position;
		uint32_t section_header_table_position;
		uint32_t flags; //Not used in x86 ELFs
		uint16_t header_size;
		uint16_t program_header_table_entry_size;
		uint16_t program_header_table_entries;
		uint16_t section_header_table_entry_size;
		uint16_t section_header_table_entries;
		uint16_t section_names_index;
	} elf32_header;

	typedef struct __attribute__((packed)) elf32_segment_header {
		uint32_t p_type;
		uint32_t p_offset;
		uint32_t p_vaddr;
		uint32_t p_paddr;
		uint32_t p_filesz;
		uint32_t p_memsz;
		uint32_t p_flags;
		uint32_t p_align;
	} elf32_segment_header;

	bool is_valid_elf_header(elf32_header* header);
	bool can_execute(elf32_header* header);
}

#endif
