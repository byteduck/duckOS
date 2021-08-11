SET(SOURCES
        main.cpp
        Client.cpp
        Display.cpp
        FontManager.cpp
        Mouse.cpp
        Window.cpp
        Server.cpp)

MAKE_PROGRAM(pond)
TARGET_LINK_LIBRARIES(pond libgraphics libduck)