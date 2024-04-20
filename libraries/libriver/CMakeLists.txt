SET(SOURCES BusConnection.cpp BusServer.cpp Endpoint.cpp IPCBuffer.cpp packet.cpp)
MAKE_LIBRARY(libriver)
TARGET_LINK_LIBRARIES(libriver libduck)