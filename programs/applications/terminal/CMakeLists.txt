SET(SOURCES main.cpp TerminalWidget.cpp)
MAKE_APP(terminal)
TARGET_LINK_LIBRARIES(terminal libterm libui)