#ifndef ELF_H
#define ELF_H

#include <kernel/kstddef.h>

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

#define ELF_SEGMENT_NULL 0
#define ELF_SEGMENT_LOAD 1
#define ELF_SEGMENT_DYNAMIC 2
#define ELF_SEGMENT_INTERPRET 3
#define ELF_SEGMENT_NOTES 4

#define ELF_SEGMENT_EXECUTABLE 1 
#define ELF_SEGMENT_WRITABLE 2
#define ELF_SEGMENT_READABLE 4

typedef struct __attribute__((packed)) elf32_segment_header {
	uint32_t type;
	uint32_t offset;
	uint32_t vaddr;
	uint32_t unused;
	uint32_t file_size;
	uint32_t mem_size;
	uint32_t flags;
	uint32_t alignment;
} elf32_segment_header;

#endif
