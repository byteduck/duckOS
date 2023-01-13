SET(SOURCES main.cpp Command.cpp Shell.cpp util.cpp)
MAKE_PROGRAM(dsh)
TARGET_LINK_LIBRARIES(dsh libtui)