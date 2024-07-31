##########
# KERNEL #
##########

set(CMAKE_CXX_STANDARD 20)

ENABLE_LANGUAGE(ASM_NASM)
SET_SOURCE_FILES_PROPERTIES(arch/i386/asm/startup.s arch/i386/asm/tasking.s arch/i386/asm/int.s arch/i386/asm/syscall.s arch/i386/asm/gdt.s arch/i386/asm/timing.s PROPERTIES LANGUAGE ASM_NASM)

SET(CMAKE_CXX_FLAGS "-ffreestanding -nostdlib -fno-rtti -fno-exceptions -Wno-write-strings -fbuiltin -nostdlib -nostdinc -nostdinc++ -std=c++2a")

# Build with debug symbols if needed
IF(ADD_KERNEL_DEBUG_SYMBOLS)
    ADD_COMPILE_OPTIONS("$<$<COMPILE_LANGUAGE:CXX>:-g>")
    ADD_COMPILE_OPTIONS("$<$<COMPILE_LANGUAGE:CXX>:-ggdb>")
ELSE()
    ADD_COMPILE_OPTIONS("-O2")
ENDIF()

SET(CMAKE_CXX_FLAGS_DEBUG "-Werror -Wno-vla-larger-than")

SET(CMAKE_ASM_NASM_COMPILER nasm)
SET(CMAKE_ASM_NASM_FLAGS "-I ${CMAKE_CURRENT_SOURCE_DIR}/ -f elf")
SET(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")

SET(KERNEL_SRCS
        kmain.cpp
        time/TimeManager.cpp
        time/TimeKeeper.cpp
        time/Time.cpp
        kstd/kstdio.cpp
        keyboard.cpp
        kstd/kstddef.cpp
        tasking/ELF.cpp
        tasking/TaskManager.cpp
        pci/PCI.cpp
        memory/liballoc.cpp
        memory/MemoryManager.cpp
        pci/PCI.cpp
        device/Device.cpp
        device/BlockDevice.cpp
        filesystem/Inode.cpp
        device/PartitionDevice.cpp
        filesystem/Filesystem.cpp
        filesystem/LinkedInode.cpp
        filesystem/ext2/Ext2Filesystem.cpp
        filesystem/ext2/Ext2BlockGroup.cpp
        filesystem/ext2/Ext2Inode.cpp
        memory/liballoc.cpp
        filesystem/VFS.cpp
        filesystem/File.cpp
        filesystem/FileDescriptor.cpp
        Result.cpp
        filesystem/InodeFile.cpp
        filesystem/InodeMetadata.cpp
        filesystem/FileBasedFilesystem.cpp
        kstd/icxxabi.cpp
        device/CharacterDevice.cpp
        device/ZeroDevice.cpp
        random.cpp
        device/RandomDevice.cpp
        device/NullDevice.cpp
        device/I8042.cpp
        device/KeyboardDevice.cpp
        device/MouseDevice.cpp
        interrupt/IRQHandler.cpp
        memory/PageDirectory.cpp
        memory/PageTable.cpp
        tasking/Process.cpp
        tasking/Thread.cpp
        tasking/Lock.cpp
        tasking/Mutex.cpp
        tasking/ProcessArgs.cpp
        tasking/Blocker.cpp
        tasking/WaitBlocker.cpp
        tasking/JoinBlocker.cpp
        tasking/BooleanBlocker.cpp
        tasking/PollBlocker.cpp
        tasking/Futex.cpp
        tasking/SleepBlocker.cpp
        tasking/FileBlockers.cpp
        tasking/Tracer.cpp
        device/VGADevice.cpp
        device/MultibootVGADevice.cpp
        memory/Stack.h
        memory/PhysicalRegion.cpp
        memory/PhysicalPage.cpp
        memory/VMObject.cpp
        memory/VMRegion.cpp
        memory/VMSpace.cpp
        memory/AnonymousVMObject.cpp
        memory/InodeVMObject.cpp
        memory/BuddyZone.cpp
        memory/Memory.cpp
        memory/KBuffer.cpp
        memory/Bytes.cpp
        CommandLine.cpp
        tasking/Signal.cpp
        filesystem/DirectoryEntry.cpp
        filesystem/Pipe.cpp
        terminal/TTYDevice.cpp
        terminal/VirtualTTY.cpp
        terminal/PTYDevice.cpp
        terminal/PTYControllerDevice.cpp
        terminal/PTYMuxDevice.cpp
        ../libraries/libterm/Terminal.cpp
        ../libraries/libterm/Line.cpp
        User.cpp
        filesystem/procfs/ProcFS.cpp
        filesystem/procfs/ProcFSInode.cpp
        filesystem/procfs/ProcFSEntry.cpp
        filesystem/procfs/ProcFSContent.cpp
        filesystem/socketfs/SocketFS.cpp
        filesystem/socketfs/SocketFSInode.cpp
        filesystem/ptyfs/PTYFS.cpp
        filesystem/ptyfs/PTYFSInode.cpp
        IO.cpp
        KernelMapper.cpp
        device/KernelLogDevice.cpp
        device/DiskDevice.cpp
		kstd/KLog.cpp
		kstd/cstring.cpp
        kstd/kstdlib.cpp
        kstd/string.cpp
        tests/KernelTest.cpp
        tests/kstd/TestMap.cpp
        tests/TestMemory.cpp
        tests/kstd/TestArc.cpp
        kstd/bits/RefCount.cpp
        kstd/Optional.cpp
        tasking/Reaper.cpp
        syscall/syscall.cpp
        syscall/access.cpp
        syscall/chdir.cpp
        syscall/chmod.cpp
        syscall/dup.cpp
        syscall/exec.cpp
        syscall/exit.cpp
        syscall/fork.cpp
        syscall/futex.cpp
        syscall/getcwd.cpp
        syscall/gettimeofday.cpp
        syscall/ioctl.cpp
        syscall/isatty.cpp
        syscall/kill.cpp
        syscall/link.cpp
        syscall/mem.cpp
        syscall/mkdir.cpp
        syscall/pid.cpp
        syscall/pipe.cpp
        syscall/poll.cpp
        syscall/ptrace.cpp
        syscall/ptsname.cpp
        syscall/read_write.cpp
        syscall/sigaction.cpp
        syscall/sleep.cpp
        syscall/socket.cpp
        syscall/stat.cpp
        syscall/thread.cpp
        syscall/truncate.cpp
        syscall/waitpid.cpp
        syscall/uname.cpp
        VMWare.cpp
        StackWalker.cpp
        net/NetworkAdapter.cpp
        net/E1000Adapter.cpp
        net/NetworkManager.cpp
        net/Socket.cpp
        net/IPSocket.cpp
        net/UDPSocket.cpp
        net/TCPSocket.cpp
        net/Router.cpp
        api/strerror.c)

IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i686")
    SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/arch/i386/kernel.ld)
    SET(KERNEL_SRCS
            ${KERNEL_SRCS}

            arch/i386/Processor.cpp
            arch/i386/idt.cpp
            arch/i386/irq.cpp
            arch/i386/isr.cpp
            arch/i386/tasking.cpp
            arch/i386/gdt.cpp
            arch/i386/kstdio.cpp

            arch/i386/asm/startup.s
            arch/i386/asm/tasking.s
            arch/i386/asm/int.s
            arch/i386/asm/syscall.s
            arch/i386/asm/gdt.s
            arch/i386/asm/timing.s

            arch/i386/device/Device.cpp
            arch/i386/device/PATADevice.cpp
            arch/i386/device/BochsVGADevice.cpp
            arch/i386/device/AC97Device.cpp

            arch/i386/time/CMOS.cpp
            arch/i386/time/PIT.cpp
            arch/i386/time/RTC.cpp)
ELSEIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
    SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/arch/aarch64/kernel.ld)
    SET(KERNEL_SRCS
            ${KERNEL_SRCS}

            arch/aarch64/Processor.cpp
            arch/aarch64/ARMTimer.cpp
            arch/aarch64/tasking.cpp
            arch/aarch64/startup.cpp
            arch/aarch64/kstdio.cpp
            arch/aarch64/MMU.cpp

            arch/aarch64/asm/startup.S
            arch/aarch64/asm/exception.S

            arch/aarch64/rpi/MiniUART.cpp
            arch/aarch64/rpi/GPIO.cpp
            arch/aarch64/rpi/Mailbox.cpp
            arch/aarch64/rpi/Framebuffer.cpp

            arch/aarch64/device/Device.cpp)
ENDIF()

add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/duckos_version.h"
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../scripts/version.sh" "${CMAKE_CURRENT_BINARY_DIR}/duckos_version.h.tmp"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/duckos_version.h.tmp" "${CMAKE_CURRENT_BINARY_DIR}/generated/duckos_version.h"
        COMMAND "${CMAKE_COMMAND}" -E remove "${CMAKE_CURRENT_BINARY_DIR}/duckos_version.h.tmp"
)
add_custom_target(generate_version_file DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generated/duckos_version.h")

ADD_EXECUTABLE(${KERNEL_EXECUTABLE_NAME} ${KERNEL_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/generated/duckos_version.h)
ADD_DEPENDENCIES(${KERNEL_EXECUTABLE_NAME} generate_version_file)
TARGET_COMPILE_DEFINITIONS(${KERNEL_EXECUTABLE_NAME} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>" DUCKOS_KERNEL "$<$<BOOL:${ADD_KERNEL_DEBUG_SYMBOLS}>:DUCKOS_KERNEL_DEBUG_SYMBOLS>")

SET_TARGET_PROPERTIES(${KERNEL_EXECUTABLE_NAME} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
TARGET_LINK_LIBRARIES(${KERNEL_EXECUTABLE_NAME} gcc)
TARGET_LINK_OPTIONS(${KERNEL_EXECUTABLE_NAME} PRIVATE LINKER:-T ${LINKER_SCRIPT} -nostdlib)

TARGET_INCLUDE_DIRECTORIES(${KERNEL_EXECUTABLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/generated)

# Install headers
FILE(GLOB_RECURSE KHEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
foreach(HEADER ${KHEADERS})
    GET_FILENAME_COMPONENT(SUBDIRECTORY ${HEADER} DIRECTORY)
    INSTALL(FILES ${HEADER} DESTINATION usr/include/kernel/${SUBDIRECTORY})
endforeach()

# Install kernel
INSTALL(TARGETS ${KERNEL_EXECUTABLE_NAME} RUNTIME DESTINATION boot)

# Make kernel map
ADD_CUSTOM_COMMAND(TARGET ${KERNEL_EXECUTABLE_NAME} COMMAND ${CMAKE_SOURCE_DIR}/scripts/kernel-map.sh ${KERNEL_EXECUTABLE_NAME})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kernel.map DESTINATION boot)