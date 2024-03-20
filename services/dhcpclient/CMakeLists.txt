SET(SOURCES main.cpp Client.cpp DHCP.cpp)

MAKE_PROGRAM(dhcpclient)
TARGET_LINK_LIBRARIES(dhcpclient libduck)