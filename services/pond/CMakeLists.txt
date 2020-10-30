SET(SOURCES
        main.cpp
        Client.cpp
        DecorationWindow.cpp
        Display.cpp
        Geometry.cpp
        Graphics.cpp
        Mouse.cpp
        Window.cpp
        Server.cpp)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-static")
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-static")

MAKE_PROGRAM(pond)
TARGET_LINK_LIBRARIES(pond libgraphics_static)