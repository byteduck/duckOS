##########
# KERNEL #
##########

set(CMAKE_CXX_STANDARD 11)

ENABLE_LANGUAGE(ASM_NASM)
SET_SOURCE_FILES_PROPERTIES(asm/startup.s asm/int.s asm/syscall.s asm/gdt.s PROPERTIES LANGUAGE ASM_NASM)

SET(CMAKE_CXX_FLAGS "-ffreestanding -O0 -Woverflow -nostdlib -fno-rtti -fno-exceptions -Wno-write-strings -fbuiltin -nostdlib -nostdinc -nostdinc++ -std=c++2a")

SET(CMAKE_ASM_NASM_COMPILER nasm)
SET(CMAKE_ASM_NASM_FLAGS "-I ${CMAKE_CURRENT_SOURCE_DIR}/ -f elf")
SET(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")

SET(KERNEL_SRCS
        asm/startup.s
        asm/int.s
        asm/syscall.s
        asm/gdt.s
        kmain.cpp
        pit.cpp
        shell.cpp
        kstdio.cpp
        keyboard.cpp
        kstddef.cpp
        tasking/elf.cpp
        tasking/TaskManager.cpp
        pci/pci.cpp
        memory/gdt.cpp
        memory/kliballoc.cpp
        memory/paging.cpp
        interrupt/idt.cpp
        interrupt/irq.cpp
        interrupt/isr.cpp
        interrupt/syscall.cpp
        filesystem/Ext2.cpp
        pci/pci.cpp
        device/ide.cpp
        device/Device.cpp
        device/BlockDevice.cpp
        device/PIODevice.cpp
        filesystem/Inode.cpp
        device/PartitionDevice.cpp
        filesystem/FileSystem.cpp
        filesystem/LinkedInode.cpp
        memory/kliballoc.cpp
        filesystem/VFS.cpp
        filesystem/File.cpp
        filesystem/FileDescriptor.cpp
        Result.cpp
        filesystem/InodeFile.cpp
        filesystem/InodeMetadata.cpp
        filesystem/FileBasedFilesystem.cpp
        icxxabi.cpp
        device/TTYDevice.cpp
        device/CharacterDevice.cpp
        device/ZeroDevice.cpp
        random.cpp
        device/RandomDevice.cpp
        device/NullDevice.cpp
        device/KeyboardDevice.cpp
        interrupt/IRQHandler.cpp
        memory/PageDirectory.cpp
        memory/PageTable.cpp
        tasking/Process.cpp
        tasking/Lock.cpp
        tasking/ProcessArgs.cpp)

SET(COMMON_SRCS
        ../common/cstring.cpp
        ../common/stdlib.cpp
        ../common/string.cpp)

ADD_EXECUTABLE(duckk32 ${KERNEL_SRCS} ${COMMON_SRCS})
TARGET_COMPILE_DEFINITIONS(duckk32 PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/kernel.ld)
SET_TARGET_PROPERTIES(duckk32 PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
SET(CMAKE_CXX_LINK_EXECUTABLE "ld <OBJECTS> -T ${LINKER_SCRIPT} -melf_i386 -Map kernel.map -o <TARGET> <LINK_LIBRARIES>")

TARGET_INCLUDE_DIRECTORIES(duckk32 PRIVATE ${CMAKE_SOURCE_DIR})

INSTALL(TARGETS duckk32 RUNTIME DESTINATION boot)