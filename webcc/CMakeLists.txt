# webcc

# Don't use any deprecated definitions (e.g., io_service).
add_compile_definitions(BOOST_ASIO_NO_DEPRECATED)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

# Adhere to GNU filesystem layout conventions.
include(GNUInstallDirs)

set(SOURCES
    base64.cc
    body.cc
    client_base.cc
    client.cc
    client_pool.cc
    client_session.cc
    common.cc
    connection.cc
    connection_base.cc
    connection_pool.cc
    globals.cc
    logger.cc
    message.cc
    message_parser.cc
    request.cc
    request_builder.cc
    request_parser.cc
    response.cc
    response_builder.cc
    response_parser.cc
    router.cc
    server.cc
    ssl_client.cc
    ssl_connection.cc
    ssl_server.cc
    string.cc
    url.cc
    utility.cc
    )

set(HEADERS
    base64.h
    body.h
    client_base.h
    client.h
    client_pool.h
    client_session.h
    common.h
    connection.h
    connection_base.h
    connection_pool.h
    globals.h
    logger.h
    message.h
    message_builder.h
    message_parser.h
    queue.h
    request.h
    request_builder.h
    request_parser.h
    response.h
    response_builder.h
    response_parser.h
    router.h
    server.h
    ssl_client.h
    ssl_connection.h
    ssl_server.h
    string.h
    url.h
    utility.h
    version.h
    view.h
    )

set(INTERNAL_SOURCES
    internal/globals.h
    )

if(WEBCC_ENABLE_GZIP)
    list(APPEND SOURCES "gzip.cc")
    list(APPEND HEADERS "gzip.h")
endif()

set(ALL_SOURCES ${SOURCES} ${HEADERS} ${INTERNAL_SOURCES})

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Add a postfix to the debug library")
mark_as_advanced(CMAKE_DEBUG_POSTFIX)

set(TARGET webcc)

add_library(${TARGET} STATIC ${ALL_SOURCES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Sources" FILES
    ${ALL_SOURCES})

# Link to pthread for Linux.
# See: https://stackoverflow.com/a/29871891
if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET} Threads::Threads)
endif()

# Boost
target_link_libraries(${TARGET} ${Boost_LIBRARIES})

# ZLIB
if(WEBCC_ENABLE_GZIP)
    # The imported target ZLIB::ZLIB could be used instead.
    target_link_libraries(${TARGET} ${ZLIB_LIBRARIES})
endif()

# OpenSSL
target_link_libraries(${TARGET} ${OPENSSL_LIBRARIES})
if(WIN32)
    target_link_libraries(${TARGET} crypt32)
endif()

# Install lib and header files.
# On Linux, if CMAKE_INSTALL_PREFIX is ~, the lib (libwebcc.a) will be installed
# to ~/lib and header files will be installed to ~/include.
install(TARGETS ${TARGET} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webcc)
install(FILES ${PROJECT_BINARY_DIR}/webcc/config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webcc)
