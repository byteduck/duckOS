SET(SOURCES AppMenu.cpp main.cpp SandbarWidget.cpp Sandbar.cpp TimeWidget.cpp)
MAKE_APP(sandbar)
TARGET_LINK_LIBRARIES(sandbar libui libduck libapp)