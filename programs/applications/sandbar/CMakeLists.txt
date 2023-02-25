SET(SOURCES AppMenu.cpp main.cpp SandbarWidget.cpp Sandbar.cpp modules/CPUModule.cpp modules/TimeModule.cpp modules/GraphModule.cpp modules/MemoryModule.cpp)
MAKE_APP(sandbar)
TARGET_LINK_LIBRARIES(sandbar libui libduck libapp libsys)