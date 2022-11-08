SET(SOURCES
        libui.cpp
        Theme.cpp
        Window.cpp
        DrawContext.cpp
        Timer.cpp
        UIException.cpp

        bits/FilePicker.cpp

        widget/Widget.cpp
        widget/Button.cpp
        widget/Cell.cpp
        widget/Checkbox.cpp
        widget/Image.cpp
        widget/Label.cpp
        widget/ProgressBar.cpp

        widget/files/FileGridView.cpp
        widget/files/FileViewBase.cpp
        widget/files/FileNavigationBar.cpp

        Menu.cpp
        widget/MenuWidget.cpp
        widget/MenuBar.cpp

        widget/ContainerView.cpp
        widget/ListView.cpp
        widget/ScrollView.cpp
        widget/TableView.cpp

        widget/layout/BoxLayout.cpp
        widget/layout/FlexLayout.cpp
        widget/layout/GridLayout.cpp
)
MAKE_LIBRARY(libui)
TARGET_LINK_LIBRARIES(libui libduck libpond libapp)