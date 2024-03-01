SET(SOURCES
        assert.c
        crt0.c
        ctype.c
        cxxabi.c
        dirent.c
        dlfcn.cpp
        errno.c
        fcntl.c
        locale.c
        poll.c
        signal.c
        stdio.c
        stdlib.c
        string.c
        strings.c
        sys/ioctl.c
        sys/shm.c
        sys/printf.c
        sys/ptrace.c
        sys/liballoc.cpp
        sys/scanf.c
        sys/socketfs.c
        sys/stat.c
        sys/status.c
        sys/syscall.c
        sys/thread.cpp
        sys/wait.c
        sys/mman.c
        sys/utsname.c
        termios.c
        time.cpp
        unistd.c
        utime.c
        ../libduck/SpinLock.cpp)

SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-nostdlib -Wall")
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-nostdlib -Wall -fno-exceptions -fno-rtti")

# Install crti.o
ADD_LIBRARY(crti STATIC crti.S)
ADD_CUSTOM_COMMAND(TARGET crti COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crti> ${CMAKE_INSTALL_PREFIX}/lib/crti.o)

# Install crtn.o
ADD_LIBRARY(crtn STATIC crtn.S)
ADD_CUSTOM_COMMAND(TARGET crtn COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crtn> ${CMAKE_INSTALL_PREFIX}/lib/crtn.o)

# Install crt0.o
ADD_LIBRARY(crt0 STATIC crt0.c)
ADD_CUSTOM_COMMAND(TARGET crt0 COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crt0> ${CMAKE_INSTALL_PREFIX}/lib/crt0.o)

# Make dynamic libc
ADD_LIBRARY(libc_dyn SHARED ${SOURCES})
SET_TARGET_PROPERTIES(libc_dyn PROPERTIES PREFIX "")
set_target_properties(libc_dyn PROPERTIES OUTPUT_NAME libc)
TARGET_LINK_DIRECTORIES(libc_dyn PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES(libc_dyn crti crt0 gcc stdc++ supc++ crtn)
ADD_CUSTOM_COMMAND(TARGET libc_dyn COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libc_dyn> ${CMAKE_INSTALL_PREFIX}/lib/libc.so)

# Make static libc
ADD_LIBRARY(libc_static STATIC ${SOURCES})
SET_TARGET_PROPERTIES(libc_static PROPERTIES PREFIX "")
set_target_properties(libc_static PROPERTIES OUTPUT_NAME libc)
TARGET_LINK_DIRECTORIES(libc_static PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES(libc_static crti crt0 gcc stdc++ supc++ crtn)
ADD_CUSTOM_COMMAND(TARGET libc_static COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libc_static> ${CMAKE_INSTALL_PREFIX}/lib/libc.a)

# Add headers target
ADD_CUSTOM_TARGET(libc_headers
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/usr/include
    COMMAND rsync -au --include '*/' --include='*.h' --include="*.hpp" --exclude='*' ${CMAKE_SOURCE_DIR}/libraries/libc/* ${CMAKE_INSTALL_PREFIX}/usr/include
)

# Combine libc_dyn and libc_static
ADD_CUSTOM_TARGET(libc)
ADD_DEPENDENCIES(libc libc_headers libc_dyn libc_static)