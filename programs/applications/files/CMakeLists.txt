SET(SOURCES main.cpp FileWidget.cpp DirectoryWidget.cpp)
MAKE_APP(files)
TARGET_LINK_LIBRARIES(files libui libapp)