SET(SOURCES
        main.cpp
        Client.cpp
        DecorationWindow.cpp
        Display.cpp
        ../../libraries/libgraphics/geometry.cpp
        ../../libraries/libgraphics/graphics.cpp
        Mouse.cpp
        Window.cpp
        Server.cpp)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

MAKE_PROGRAM(pond)
TARGET_LINK_LIBRARIES(pond libgraphics)