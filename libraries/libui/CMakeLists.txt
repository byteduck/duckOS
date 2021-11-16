SET(SOURCES
        libui.cpp
        Theme.cpp
        Window.cpp
        DrawContext.cpp
        UIException.cpp

        widget/Widget.cpp
        widget/Button.cpp
        widget/Checkbox.cpp
        widget/Image.cpp
        widget/Label.cpp
        widget/ProgressBar.cpp

        widget/ListScrollable.cpp
        widget/Scrollable.cpp
        widget/ScrollContainer.cpp
        widget/ScrollView.cpp

        widget/layout/BoxLayout.cpp
        widget/layout/FlexLayout.cpp
        widget/layout/GridLayout.cpp
)
MAKE_LIBRARY(libui)
TARGET_LINK_LIBRARIES(libui libpond libapp)