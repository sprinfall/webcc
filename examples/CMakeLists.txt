# Examples

# Common libraries to link for examples.
set(EXAMPLE_LIBS
    webcc
    Boost::filesystem
    Boost::system
    Boost::date_time
    "${CMAKE_THREAD_LIBS_INIT}")

if(WEBCC_ENABLE_SSL)
    set(EXAMPLE_LIBS ${EXAMPLE_LIBS} ${OPENSSL_LIBRARIES})

    if(WIN32)
        set(EXAMPLE_LIBS ${EXAMPLE_LIBS} crypt32)
    endif()
endif()

if(WEBCC_ENABLE_GZIP)
    if(WIN32)
        set(EXAMPLE_LIBS ${EXAMPLE_LIBS} zlibstatic)
    else()
        set(EXAMPLE_LIBS ${EXAMPLE_LIBS} ${ZLIB_LIBRARIES})
    endif()
endif()

if(UNIX)
    # Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
    set(EXAMPLE_LIBS ${EXAMPLE_LIBS} ${CMAKE_DL_LIBS})
endif()

add_executable(concurrency_test concurrency_test.cc)
target_link_libraries(concurrency_test ${EXAMPLE_LIBS})

add_executable(client_basics client_basics.cc)
target_link_libraries(client_basics ${EXAMPLE_LIBS})

if(WEBCC_ENABLE_SSL)
    add_executable(github_client github_client.cc)
    target_link_libraries(github_client ${EXAMPLE_LIBS} jsoncpp)
endif()

add_executable(hello_world_server hello_world_server.cc)
target_link_libraries(hello_world_server ${EXAMPLE_LIBS})

add_executable(static_file_server static_file_server.cc)
target_link_libraries(static_file_server ${EXAMPLE_LIBS})

add_executable(file_downloader file_downloader.cc)
target_link_libraries(file_downloader ${EXAMPLE_LIBS})

add_executable(server_states server_states.cc)
target_link_libraries(server_states ${EXAMPLE_LIBS})

add_executable(form_client form_client.cc)
target_link_libraries(form_client ${EXAMPLE_LIBS})

add_executable(form_server form_server.cc)
target_link_libraries(form_server ${EXAMPLE_LIBS})

add_subdirectory(book_server)
add_subdirectory(book_client)

if(WIN32)
    add_executable(url_unicode url_unicode.cc encoding.cc encoding.h)
    target_link_libraries(url_unicode ${EXAMPLE_LIBS})
endif()
