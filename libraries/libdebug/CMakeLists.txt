SET(SOURCES Debugger.cpp)
MAKE_LIBRARY(libdebug)
TARGET_LINK_LIBRARIES(libdebug libtui)