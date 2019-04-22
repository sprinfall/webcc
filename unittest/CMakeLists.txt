# Unit test

set(UT_SRCS
    base64_unittest.cc
    parser_unittest.cc
    rest_service_manager_unittest.cc
    url_unittest.cc
    )

set(UT_TARGET_NAME webcc_unittest)

# Common libraries to link.
set(UT_LIBS webcc ${Boost_LIBRARIES} "${CMAKE_THREAD_LIBS_INIT}")

if(WEBCC_ENABLE_SSL)
    set(UT_LIBS ${UT_LIBS} ${OPENSSL_LIBRARIES} crypt32)
endif()

if(WEBCC_ENABLE_GZIP)
    if(WIN32)
        set(UT_LIBS ${UT_LIBS} zlibstatic)
    else()
        set(UT_LIBS ${UT_LIBS} ${ZLIB_LIBRARIES})
    endif()
endif()

if(UNIX)
    # Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
    set(UT_LIBS ${UT_LIBS} ${CMAKE_DL_LIBS})
endif()

add_executable(${UT_TARGET_NAME} ${UT_SRCS})
target_link_libraries(${UT_TARGET_NAME} webcc gtest ${UT_LIBS})

add_test(${UT_TARGET_NAME} ${UT_TARGET_NAME})
