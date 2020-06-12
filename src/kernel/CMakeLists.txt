##########
# KERNEL #
##########

set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

ENABLE_LANGUAGE(ASM_NASM)
SET_SOURCE_FILES_PROPERTIES(asm/startup.s PROPERTIES LANGUAGE ASM_NASM)

SET(CMAKE_CXX_FLAGS "-ffreestanding -O0 -Woverflow -m32 -nostdlib -fno-rtti -fno-exceptions -Wno-write-strings")
SET(CMAKE_C_FLAGS "-ffreestanding -O0 -std=gnu99 -Woverflow -m32")

SET(CMAKE_ASM_NASM_COMPILER ${TOOLCHAIN_PREFIX}nasm)
SET(CMAKE_ASM_NASM_FLAGS "-I ${CMAKE_CURRENT_SOURCE_DIR}/ -f elf")
SET(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")

SET(SOURCE_FILES asm/startup.s kmain.cpp pit.cpp shell.cpp kstdio.cpp keyboard.cpp kstddef.cpp device/ata.cpp tasking/elf.cpp tasking/tasking.cpp tasking/tss.cpp pci/pci.cpp memory/gdt.cpp memory/kliballoc.cpp memory/paging.cpp interrupt/idt.cpp interrupt/irq.cpp interrupt/isr.cpp interrupt/syscall.cpp filesystem/Ext2.cpp pci/pci.cpp device/ide.cpp device/ide.h device/Device.cpp device/Device.h device/BlockDevice.cpp device/BlockDevice.h device/PIODevice.cpp device/PIODevice.h filesystem/Inode.cpp filesystem/Inode.h device/PartitionDevice.cpp device/PartitionDevice.h filesystem/FileSystem.cpp filesystem/LinkedInode.cpp filesystem/LinkedInode.h memory/kliballoc.cpp memory/kliballoc.h memory/MemoryBitmap.hpp filesystem/VFS.cpp filesystem/VFS.h filesystem/File.cpp filesystem/File.h ../common/string.h ../common/cstring.h ../common/stdlib.h ../common/cstring.cpp ../common/stdlib.cpp ../common/string.cpp filesystem/FileDescriptor.cpp filesystem/FileDescriptor.h Result.cpp Result.hpp filesystem/InodeFile.cpp filesystem/InodeFile.h filesystem/InodeMetadata.cpp filesystem/InodeMetadata.h ../common/defines.h filesystem/FileBasedFilesystem.cpp filesystem/FileBasedFilesystem.h filesystem/DirectoryEntry.cpp filesystem/DirectoryEntry.h ../common/vector.hpp icxxabi.cpp icxxabi.h device/TTYDevice.cpp device/TTYDevice.h device/CharacterDevice.cpp device/CharacterDevice.h device/ZeroDevice.cpp device/ZeroDevice.h random.cpp random.h device/RandomDevice.cpp device/RandomDevice.h device/NullDevice.cpp device/NullDevice.h ../common/circular_queue.hpp device/KeyboardDevice.cpp device/KeyboardDevice.h interrupt/IRQHandler.cpp interrupt/IRQHandler.h)
ADD_EXECUTABLE(duckk32 ${SOURCE_FILES})

SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/kernel.ld)
SET_TARGET_PROPERTIES(duckk32 PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
SET(CMAKE_CXX_LINK_EXECUTABLE "ld <OBJECTS> -T ${LINKER_SCRIPT} -melf_i386 -Map kernel.map -o <TARGET> <LINK_LIBRARIES>")

TARGET_INCLUDE_DIRECTORIES(duckk32 PRIVATE ${CMAKE_SOURCE_DIR})