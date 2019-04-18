# Auto test

set(AT_SRCS
    http_client_autotest.cc
    )

set(AT_TARGET_NAME webcc_autotest)

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

add_executable(${AT_TARGET_NAME} ${AT_SRCS})
target_link_libraries(${AT_TARGET_NAME} webcc jsoncpp gtest ${TEST_LIBS})
