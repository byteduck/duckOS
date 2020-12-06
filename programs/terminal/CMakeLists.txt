SET(SOURCES main.cpp TerminalWidget.cpp ../../common/terminal/Terminal.cpp)
MAKE_PROGRAM(terminal)
TARGET_LINK_LIBRARIES(terminal libui)