SET(SOURCES
        libui.cpp
        Theme.cpp
        Window.cpp
        DrawContext.cpp

        widget/Widget.cpp
        widget/Button.cpp
        widget/Checkbox.cpp
        widget/Label.cpp
        widget/StackView.cpp
)
MAKE_LIBRARY(libui)
TARGET_LINK_LIBRARIES(libui libpond)