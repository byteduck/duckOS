SET(SOURCES main.cpp ProcessListWidget.cpp MemoryUsageWidget.cpp ProcessManager.cpp ProcessInspectorWidget.cpp ProcessMemoryLayoutWidget.cpp)
MAKE_APP(monitor)
TARGET_LINK_LIBRARIES(monitor libui libsys)