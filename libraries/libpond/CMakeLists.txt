SET(SOURCES packet.cpp Event.cpp Context.cpp Window.cpp)
MAKE_LIBRARY(libpond)
TARGET_LINK_LIBRARIES(libpond libgraphics libriver libapp)