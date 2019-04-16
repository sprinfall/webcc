# Tests

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

add_executable(test_logger test_logger.cc)
target_link_libraries(test_logger ${TEST_LIBS})

add_executable(test_http_client test_http_client.cc)
target_link_libraries(test_http_client gtest jsoncpp ${TEST_LIBS})
