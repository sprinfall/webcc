# Unit test

set(UT_TARGET_NAME unittest)

set(UT_SRCS
    base64_unittest.cc
    body_unittest.cc
    request_parser_unittest.cc
    response_builder_unittest.cc
    router_unittest.cc
    string_unittest.cc
    url_unittest.cc
    )

# Common libraries to link.
set(UT_LIBS
    webcc
    GTest::GTest
    GTest::Main)

if(UNIX)
    # Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
    set(UT_LIBS ${UT_LIBS} ${CMAKE_DL_LIBS})
endif()

add_executable(${UT_TARGET_NAME} ${UT_SRCS})
target_link_libraries(${UT_TARGET_NAME} ${UT_LIBS})
set_target_properties(${UT_TARGET_NAME} PROPERTIES FOLDER "Tests")

add_test(${UT_TARGET_NAME} ${UT_TARGET_NAME})
