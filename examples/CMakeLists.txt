# Examples

# Common libraries to link for examples.
set(EXAMPLE_LIBS webcc ${Boost_LIBRARIES} "${CMAKE_THREAD_LIBS_INIT}")

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

set(REST_BOOK_SRCS
    common/book.cc
    common/book.h
    common/book_json.cc
    common/book_json.h
    )

add_executable(client_basics client_basics.cc)
target_link_libraries(client_basics ${EXAMPLE_LIBS})

if(WEBCC_ENABLE_SSL)
    add_executable(github_client github_client.cc)
    target_link_libraries(github_client ${EXAMPLE_LIBS} jsoncpp)
endif()

add_executable(rest_book_server rest_book_server.cc ${REST_BOOK_SRCS})
target_link_libraries(rest_book_server ${EXAMPLE_LIBS} jsoncpp)

add_executable(rest_book_client rest_book_client.cc ${REST_BOOK_SRCS})
target_link_libraries(rest_book_client ${EXAMPLE_LIBS} jsoncpp)

add_executable(file_upload_client file_upload_client.cc)
target_link_libraries(file_upload_client ${EXAMPLE_LIBS})

add_executable(file_upload_server file_upload_server.cc)
target_link_libraries(file_upload_server ${EXAMPLE_LIBS})
