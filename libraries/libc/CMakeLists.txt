SET(SOURCES
        assert.c
        crt0.c
        ctype.c
        cxxabi.c
        dirent.c
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
        sys/mem.c
        sys/printf.c
        sys/liballoc.c
        sys/socketfs.c
        sys/stat.c
        sys/syscall.c
        sys/thread.cpp
        sys/wait.c
        termios.c
        time.c
        unistd.c)

SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-nostdlib -Wall")
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-nostdlib -Wall")

# Install crt0.o
ADD_LIBRARY(crt0 STATIC crt0.c)
ADD_CUSTOM_COMMAND(TARGET crt0 COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crt0> ${CMAKE_INSTALL_PREFIX}/usr/lib/crt0.o)

# Make dynamic libc
ADD_LIBRARY(libc_dyn ${SOURCES})
SET_TARGET_PROPERTIES(libc_dyn PROPERTIES PREFIX "")
set_target_properties(libc_dyn PROPERTIES OUTPUT_NAME libc)
TARGET_LINK_DIRECTORIES(libc_dyn PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES(libc_dyn crt0 gcc stdc++ supc++)
ADD_CUSTOM_COMMAND(TARGET libc_dyn COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libc_dyn> ${CMAKE_INSTALL_PREFIX}/usr/lib/libc.so)

# Make static libc
ADD_LIBRARY(libc_static STATIC ${SOURCES})
SET_TARGET_PROPERTIES(libc_static PROPERTIES PREFIX "")
set_target_properties(libc_static PROPERTIES OUTPUT_NAME libc)
TARGET_LINK_DIRECTORIES(libc_static PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES(libc_static crt0 gcc stdc++ supc++)
ADD_CUSTOM_COMMAND(TARGET libc_static COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libc_static> ${CMAKE_INSTALL_PREFIX}/usr/lib/libc.a)

# Combine libc_dyn and libc_static
ADD_CUSTOM_TARGET(libc)
ADD_DEPENDENCIES(libc libc_dyn libc_static)

# Install headers
FILE(GLOB_RECURSE LIBHEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
foreach(HEADER ${LIBHEADERS})
    GET_FILENAME_COMPONENT(SUBDIRECTORY ${HEADER} DIRECTORY)
    INSTALL(FILES ${HEADER} DESTINATION usr/include)
endforeach()
