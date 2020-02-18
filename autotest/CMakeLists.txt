# Automation test

set(AT_SRCS
    client_autotest.cc
    client_timeout_autotest.cc
    main.cc
    )

set(AT_TARGET_NAME webcc_autotest)

# Common libraries to link.
set(AT_LIBS
    webcc
    jsoncpp
    GTest::GTest
    "${CMAKE_THREAD_LIBS_INIT}")

if(WEBCC_ENABLE_SSL)
    set(AT_LIBS ${AT_LIBS} ${OPENSSL_LIBRARIES})

    if(WIN32)
        set(AT_LIBS ${AT_LIBS} crypt32)
    endif()
endif()

if(WEBCC_ENABLE_GZIP)
    if(WIN32)
        set(AT_LIBS ${AT_LIBS} zlibstatic)
    else()
        set(AT_LIBS ${AT_LIBS} ${ZLIB_LIBRARIES})
    endif()
endif()

if(UNIX)
    # Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
    set(AT_LIBS ${AT_LIBS} ${CMAKE_DL_LIBS})
endif()

add_executable(${AT_TARGET_NAME} ${AT_SRCS})
target_link_libraries(${AT_TARGET_NAME} ${AT_LIBS})
