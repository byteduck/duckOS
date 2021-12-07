SET(SOURCES main.cpp ProcessListWidget.cpp)
MAKE_APP(monitor)
TARGET_LINK_LIBRARIES(monitor libui libsys)