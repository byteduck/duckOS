SET(SOURCES libui.cpp Button.cpp StackView.cpp Theme.cpp Widget.cpp Window.cpp)
MAKE_LIBRARY(libui)
TARGET_LINK_LIBRARIES(libui libpond)