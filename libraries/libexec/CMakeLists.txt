SET(SOURCES Object.cpp Loader.cpp dlfunc.cpp)
MAKE_LIBRARY(libexec)
TARGET_LINK_LIBRARIES(libexec libduck)
TARGET_LINK_LIBRARIES(libexec_static libduck_static)