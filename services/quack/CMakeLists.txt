SET(SOURCES main.cpp SoundServer.cpp Client.cpp)

MAKE_PROGRAM(quack)
TARGET_LINK_LIBRARIES(quack libduck libriver libsound)