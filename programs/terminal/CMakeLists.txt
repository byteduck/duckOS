SET(SOURCES main.cpp ../../common/terminal/Terminal.cpp)
MAKE_PROGRAM(terminal)
TARGET_LINK_LIBRARIES(terminal libpond)