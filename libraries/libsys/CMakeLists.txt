SET(SOURCES CPU.cpp Memory.cpp Process.cpp)
MAKE_LIBRARY(libsys)
TARGET_LINK_LIBRARIES(libsys libduck libapp)