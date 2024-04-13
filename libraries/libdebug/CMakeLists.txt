SET(SOURCES Debugger.cpp Info.cpp LiveDebugger.cpp)
MAKE_LIBRARY(libdebug)
TARGET_LINK_LIBRARIES(libdebug libduck libsys libexec)