SET(SOURCES main.cpp ViewerWidget.cpp ViewerAudioWidget.cpp)
MAKE_APP(viewer)
TARGET_LINK_LIBRARIES(viewer libui libsound)