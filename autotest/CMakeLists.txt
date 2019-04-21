# Automation test

set(AT_SRCS
    client_autotest.cc
    )

set(AT_TARGET_NAME webcc_autotest)

# Common libraries to link.
set(AT_LIBS webcc ${Boost_LIBRARIES} "${CMAKE_THREAD_LIBS_INIT}")

if(WEBCC_ENABLE_SSL)
    set(AT_LIBS ${AT_LIBS} ${OPENSSL_LIBRARIES})
endif()

if(WIN32)
    # TODO: crypt32 should be ssl related.
    set(AT_LIBS ${AT_LIBS} zlibstatic crypt32)
else()
    set(AT_LIBS ${AT_LIBS} ${ZLIB_LIBRARIES})
endif()

if(UNIX)
    # Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
    set(AT_LIBS ${AT_LIBS} ${CMAKE_DL_LIBS})
endif()

add_executable(${AT_TARGET_NAME} ${AT_SRCS})
target_link_libraries(${AT_TARGET_NAME} webcc jsoncpp gtest ${AT_LIBS})
