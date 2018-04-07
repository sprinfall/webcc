if(UNIX)
    add_definitions(-std=c++11)
endif()

set(SRCS
    gtest-all.cc
    gtest_main.cc
    )

#add_definitions(-DLIBCONFIG_EXPORTS -DYY_NO_UNISTD_H -DYY_USE_CONST)

add_library(gtest ${SRCS})

#if(MSVC)
#set_target_properties(libconfig PROPERTIES COMPILE_FLAGS "/wd4290")
#endif()

install(TARGETS gtest DESTINATION .)

