# Examples

# Common libraries to link for examples.
set(EXAMPLE_LIBS webcc)

if(UNIX)
    # Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
    set(EXAMPLE_LIBS ${EXAMPLE_LIBS} ${CMAKE_DL_LIBS})
endif()

set(SIMPLE_EXAMPLES
    concurrency_test
    client_basics
    heartbeat_client
    google_client
    github_client
    hello_world_server
    static_file_server
    file_downloader
    server_states
    form_client
    form_server
    form_urlencoded_client
    ssl_hello_client
    ssl_hello_server
    ssl_certificate
    )

if(WIN32)
    list(APPEND SIMPLE_EXAMPLES url_unicode)
endif()

foreach(example ${SIMPLE_EXAMPLES})
    add_executable(${example} ${example}.cc)
    target_link_libraries(${example} ${EXAMPLE_LIBS})
    set_target_properties(${example} PROPERTIES FOLDER "Examples")
endforeach()

target_link_libraries(github_client jsoncpp)

add_subdirectory(book_server)
add_subdirectory(book_client)

if(WEBCC_BUILD_QT_EXAMPLES)
    add_subdirectory(qt_app_server)
endif()
