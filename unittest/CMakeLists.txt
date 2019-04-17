# Unit test

set(UT_SRCS
    base64_test.cc
    http_parser_test.cc
    rest_service_manager_test.cc
    url_test.cc
    )

set(UT_TARGET_NAME webcc_unittest)

# Common libraries to link.
set(TEST_LIBS webcc ${Boost_LIBRARIES} "${CMAKE_THREAD_LIBS_INIT}")

if(WEBCC_ENABLE_SSL)
    set(TEST_LIBS ${TEST_LIBS} ${OPENSSL_LIBRARIES})
endif()

if(WIN32)
    set(TEST_LIBS ${TEST_LIBS} zlibstatic crypt32)
else()
    set(TEST_LIBS ${TEST_LIBS} ${ZLIB_LIBRARIES})
endif()

if(UNIX)
    # Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
    set(TEST_LIBS ${TEST_LIBS} ${CMAKE_DL_LIBS})
endif()

add_executable(${UT_TARGET_NAME} ${UT_SRCS})
target_link_libraries(${UT_TARGET_NAME} webcc gtest ${TEST_LIBS})

add_test(${UT_TARGET_NAME} ${UT_TARGET_NAME})
