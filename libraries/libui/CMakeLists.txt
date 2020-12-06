SET(SOURCES libui.cpp Widget.cpp Window.cpp)
MAKE_LIBRARY(libui)
TARGET_LINK_LIBRARIES(libui libpond)