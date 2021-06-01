SET(SOURCES main.cpp TerminalWidget.cpp)
MAKE_PROGRAM(terminal)
TARGET_LINK_LIBRARIES(terminal libterm libui)