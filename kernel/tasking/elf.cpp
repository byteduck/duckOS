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

#include <kernel/tasking/elf.h>
#include <common/defines.h>
#include <kernel/filesystem/VFS.h>
#include <kernel/memory/memory.h>
#include "TaskManager.h"

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