set(SRCS
    client_timeout_autotest.cc
    main.cc
    )

# Common libraries to link.
set(LIBS
    webcc
    jsoncpp
    GTest::GTest
    )

if(UNIX)
    # Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
    set(LIBS ${LIBS} ${CMAKE_DL_LIBS})
endif()

set(TARGET_NAME client_timeout_autotest)

add_executable(${TARGET_NAME} ${SRCS})
target_link_libraries(${TARGET_NAME} ${LIBS})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests")
