SET(SOURCES main.cpp Uxn.cpp devices/Device.cpp devices/ConsoleDevice.cpp devices/ScreenDevice.cpp)
MAKE_PROGRAM(uxn)
TARGET_LINK_LIBRARIES(uxn libui libduck)