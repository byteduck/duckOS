SET(SOURCES App.cpp)
MAKE_LIBRARY(libapp)
TARGET_LINK_LIBRARIES(libapp libgraphics libduck)