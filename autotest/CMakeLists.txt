# Automation test

set(AT_SRCS
    client_autotest.cc
    client_timeout_autotest.cc
    main.cc
    )

# Common libraries to link.
set(AT_LIBS
    webcc
    jsoncpp
    GTest::GTest)

if(UNIX)
    # Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
    set(AT_LIBS ${AT_LIBS} ${CMAKE_DL_LIBS})
endif()

set(AT_TARGET_NAME autotest)

add_executable(${AT_TARGET_NAME} ${AT_SRCS})
target_link_libraries(${AT_TARGET_NAME} ${AT_LIBS})
set_target_properties(${AT_TARGET_NAME} PROPERTIES FOLDER "Tests")
