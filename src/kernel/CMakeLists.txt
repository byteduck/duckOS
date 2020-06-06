##########
# KERNEL #
##########

set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/kernel)

ENABLE_LANGUAGE(ASM_NASM)
SET_SOURCE_FILES_PROPERTIES(asm/startup.s PROPERTIES LANGUAGE ASM_NASM)

SET(CMAKE_CXX_FLAGS "-ffreestanding -O0 -Woverflow -m32 -nostdlib -fno-rtti -fno-exceptions -Wno-write-strings")
SET(CMAKE_C_FLAGS "-ffreestanding -O0 -std=gnu99 -Woverflow -m32")

SET(CMAKE_ASM_NASM_COMPILER ${TOOLCHAIN_PREFIX}nasm)
SET(CMAKE_ASM_NASM_FLAGS "-I ${CMAKE_CURRENT_SOURCE_DIR}/ -f elf")
SET(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")

SET(SOURCE_FILES asm/startup.s kmain.cpp pit.cpp shell.cpp stdio.cpp keyboard.cpp common.cpp device/ata.cpp tasking/elf.cpp tasking/tasking.cpp tasking/tss.cpp pci/pci.cpp memory/gdt.cpp memory/heap.cpp memory/paging.cpp interrupt/idt.cpp interrupt/irq.cpp interrupt/isr.cpp interrupt/syscall.cpp filesystem/Ext2.cpp pci/pci.cpp device/ide.cpp device/ide.h device/Device.cpp device/Device.h device/BlockDevice.cpp device/BlockDevice.h device/PIODevice.cpp device/PIODevice.h filesystem/Inode.cpp filesystem/Inode.h device/PartitionDevice.cpp device/PartitionDevice.h filesystem/FileSystem.cpp filesystem/InodeRef.cpp filesystem/InodeRef.h)
ADD_EXECUTABLE(duckk32 ${SOURCE_FILES})

SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel.ld)
SET_TARGET_PROPERTIES(duckk32 PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
SET(CMAKE_CXX_LINK_EXECUTABLE "ld <OBJECTS> -T ${LINKER_SCRIPT} -melf_i386 -Map kernel.map -o <TARGET> <LINK_LIBRARIES>")

TARGET_INCLUDE_DIRECTORIES(duckk32 PRIVATE ${CMAKE_SOURCE_DIR})