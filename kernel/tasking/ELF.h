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

#pragma once

#include <kernel/kstd/kstddef.h>
#include <kernel/kstd/kstdio.h>

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

#include <kernel/kstd/vector.hpp>
#include <kernel/Result.hpp>
#include <kernel/kstd/string.h>
#include <kernel/memory/VMSpace.h>

class FileDescriptor;
class User;
class PageDirectory;
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

	class ElfInfo {
	public:
		kstd::shared_ptr<elf32_header> header;
		kstd::vector<elf32_segment_header> segments;
		kstd::shared_ptr<FileDescriptor> fd;
		kstd::string interpreter;
	};

	/**
	 * Reads the elf32 header of the given file descriptor.
	 * @param fd The file descriptor of the executable. Will be seeked.
	 * @return The elf32 header, or an error if the file couldn't be read or isn't a valid ELF.
	 */
	ResultRet<elf32_header*> read_header(FileDescriptor& fd);

	/**
	 * Reads the program headers of an ELF file.
	 * @param fd The file descriptor of the ELF file.
	 * @param header The elf32_header of the file.
	 * @return An error or a vector containing the segment headers.
	 */
	ResultRet<kstd::vector<ELF::elf32_segment_header>> read_program_headers(FileDescriptor& fd, elf32_header* header);

	/**
	 * Reads the INTERP section of an ELF file.
	 * @param fd The file descriptor of the ELF file.
	 * @param headers The program headers (loaded by ELF::read_program_headers)
	 * @return An error or the INTERP section. -ENOENT if there is no INTERP section.
	 */
	ResultRet<kstd::string> read_interp(FileDescriptor& fd, kstd::vector<elf32_segment_header>& headers);

	/**
	 * Loads the sections of an ELF file into memory.
	 * @param fd The file descriptor of the ELF file.
	 * @param headers The program headers (loaded by ELF::read_program_headers)
	 * @param page_directory The page directory to load the program into.
	 * @return An error or the program break.
	 */
	ResultRet<kstd::vector<Ptr<VMRegion>>> load_sections(FileDescriptor& fd, kstd::vector<elf32_segment_header>& headers, const Ptr<VMSpace>& vm_space);

	/**
	 * Gets information about an ELF executable.
	 * @param fd The file descriptor of the executable. Will be seeked.
	 * @param user The user executing the executable.
	 * @param interpreter If not empty, ENOEXEC will be returned if the executable requests an interpreter.
	 * @return Information about the ELF executable or an error.
	 */
	 ResultRet<ElfInfo> read_info(const kstd::shared_ptr<FileDescriptor>& fd, User& user, kstd::string interpreter = kstd::string());
}
