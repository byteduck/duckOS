SET(SOURCES
        Line.cpp
        Terminal.cpp
)
MAKE_LIBRARY(libterm)
TARGET_LINK_LIBRARIES(libterm libduck)