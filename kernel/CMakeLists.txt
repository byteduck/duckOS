##########
# KERNEL #
##########

set(CMAKE_CXX_STANDARD 11)

ENABLE_LANGUAGE(ASM_NASM)
SET_SOURCE_FILES_PROPERTIES(asm/startup.s asm/tasking.s asm/int.s asm/syscall.s asm/gdt.s PROPERTIES LANGUAGE ASM_NASM)

SET(CMAKE_CXX_FLAGS "-ffreestanding -O2 -nostdlib -fno-rtti -fno-exceptions -Wno-write-strings -fbuiltin -nostdlib -nostdinc -nostdinc++ -std=c++2a")

SET(CMAKE_ASM_NASM_COMPILER nasm)
SET(CMAKE_ASM_NASM_FLAGS "-I ${CMAKE_CURRENT_SOURCE_DIR}/ -f elf")
SET(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")

SET(KERNEL_SRCS
        asm/startup.s
        asm/tasking.s
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
        memory/Memory.cpp
        interrupt/idt.cpp
        interrupt/irq.cpp
        interrupt/isr.cpp
        interrupt/syscall.cpp
        pci/pci.cpp
        device/Device.cpp
        device/BlockDevice.cpp
        filesystem/Inode.cpp
        device/PartitionDevice.cpp
        filesystem/Filesystem.cpp
        filesystem/LinkedInode.cpp
        filesystem/ext2/Ext2Filesystem.cpp
        filesystem/ext2/Ext2BlockGroup.cpp
        filesystem/ext2/Ext2Inode.cpp
        memory/kliballoc.cpp
        filesystem/VFS.cpp
        filesystem/File.cpp
        filesystem/FileDescriptor.cpp
        Result.cpp
        filesystem/InodeFile.cpp
        filesystem/InodeMetadata.cpp
        filesystem/FileBasedFilesystem.cpp
        icxxabi.cpp
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
        tasking/SpinLock.cpp
        tasking/ProcessArgs.cpp
        tasking/Blocker.cpp
        tasking/WaitBlocker.cpp
        tasking/BooleanBlocker.cpp
        device/VGADevice.cpp
        device/BochsVGADevice.cpp
        device/MultibootVGADevice.cpp
        memory/MemoryRegion.cpp
        memory/MemoryMap.cpp
        memory/LinkedMemoryRegion.cpp
        device/PATADevice.cpp
        CommandLine.cpp
        tasking/Signal.cpp
        filesystem/DirectoryEntry.cpp
        filesystem/Pipe.cpp
        terminal/TTYDevice.cpp
        terminal/Terminal.cpp
        User.cpp
        filesystem/procfs/ProcFS.cpp
        filesystem/procfs/ProcFSInode.cpp
        filesystem/procfs/ProcFSEntry.cpp)

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