SET(SOURCES main.cpp)
MAKE_PROGRAM(sandbar)
TARGET_LINK_LIBRARIES(sandbar libui libduck libapp)