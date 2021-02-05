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
	hello_world_server
	static_file_server
	file_downloader
	server_states
	form_client
	form_server
	form_urlencoded_client
	)

foreach(example ${SIMPLE_EXAMPLES})
	add_executable(${example} ${example}.cc)
	target_link_libraries(${example} ${EXAMPLE_LIBS})
	set_target_properties(${example} PROPERTIES FOLDER "Examples")
endforeach()

if(WEBCC_ENABLE_SSL)
    add_executable(github_client github_client.cc)
    target_link_libraries(github_client ${EXAMPLE_LIBS} jsoncpp)
	set_target_properties(github_client PROPERTIES FOLDER "Examples")
endif()

if(WIN32)
    add_executable(url_unicode url_unicode.cc encoding.cc encoding.h)
    target_link_libraries(url_unicode ${EXAMPLE_LIBS})
	set_target_properties(url_unicode PROPERTIES FOLDER "Examples")
endif()

add_subdirectory(book_server)
add_subdirectory(book_client)

if(BUILD_QT_EXAMPLES)
    add_subdirectory(qt_app_server)
endif()
