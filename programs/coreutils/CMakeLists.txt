function(MAKE_COREUTIL PROGNAME)
    SET(SOURCES ${PROGNAME}.cpp)
    MAKE_PROGRAM(${PROGNAME})
endfunction()

MAKE_COREUTIL(open)
TARGET_LINK_LIBRARIES(open libapp)
MAKE_COREUTIL(cat)
TARGET_LINK_LIBRARIES(cat libduck)
MAKE_COREUTIL(chmod)
MAKE_COREUTIL(chown)
MAKE_COREUTIL(cp)
MAKE_COREUTIL(echo)
MAKE_COREUTIL(free)
TARGET_LINK_LIBRARIES(free libsys)
MAKE_COREUTIL(ln)
MAKE_COREUTIL(ls)
TARGET_LINK_LIBRARIES(ls libduck)
MAKE_COREUTIL(mkdir)
MAKE_COREUTIL(mv)
MAKE_COREUTIL(ps)
TARGET_LINK_LIBRARIES(ps libsys)
MAKE_COREUTIL(pwd)
MAKE_COREUTIL(rm)
MAKE_COREUTIL(rmdir)
MAKE_COREUTIL(touch)
MAKE_COREUTIL(truncate)
MAKE_COREUTIL(play)
TARGET_LINK_LIBRARIES(play libsound)
MAKE_COREUTIL(date)
MAKE_COREUTIL(uname)
TARGET_LINK_LIBRARIES(uname libduck)
MAKE_COREUTIL(kill)
TARGET_LINK_LIBRARIES(kill libduck)
MAKE_COREUTIL(profile)
TARGET_LINK_LIBRARIES(profile libdebug)