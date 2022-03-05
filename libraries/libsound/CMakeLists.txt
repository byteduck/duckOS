SET(SOURCES SampleBuffer.cpp Connection.cpp WavReader.cpp)
MAKE_LIBRARY(libsound)
TARGET_LINK_LIBRARIES(libsound libduck libriver)