SET(SOURCES packet.cpp PEvent.cpp PContext.cpp PWindow.cpp)
MAKE_LIBRARY(libpond)
TARGET_LINK_LIBRARIES(libpond libgraphics)