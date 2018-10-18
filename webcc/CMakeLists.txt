# Don't use any deprecated definitions (e.g., io_service).
add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Adhere to GNU filesystem layout conventions.
include(GNUInstallDirs)

set(HEADERS
    version.h
    globals.h
    http_async_client.h
    http_client.h
    http_session.h
    http_message.h
    http_parser.h
    http_request.h
    http_request_handler.h
    http_request_parser.h
    http_response.h
    http_response_parser.h
    http_server.h
    logger.h
    queue.h
    basic_rest_client.h
    rest_async_client.h
    rest_client.h
    rest_request_handler.h
    rest_server.h
    rest_service.h
    rest_service_manager.h
    url.h
    utility.h
    )

set(SOURCES
    globals.cc
    http_async_client.cc
    http_client.cc
    http_session.cc
    http_message.cc
    http_parser.cc
    http_request.cc
    http_request_handler.cc
    http_request_parser.cc
    http_response.cc
    http_response_parser.cc
    http_server.cc
    logger.cc
    rest_async_client.cc
    rest_request_handler.cc
    rest_service_manager.cc
    rest_service.cc
    url.cc
    utility.cc
    )

if(WEBCC_ENABLE_SSL)
    set(HEADERS ${HEADERS}
        http_ssl_client.h
        rest_ssl_client.h
        )

    set(SOURCES ${SOURCES}
        http_ssl_client.cc
        )
endif()

if(WEBCC_ENABLE_SOAP)
    set(SOAP_HEADERS
        soap_async_client.h
        soap_client.h
        soap_globals.h
        soap_message.h
        soap_parameter.h
        soap_response.h
        soap_request.h
        soap_request_handler.h
        soap_server.h
        soap_service.h
        soap_xml.h
        )

    set(SOAP_SOURCES
        soap_async_client.cc
        soap_client.cc
        soap_globals.cc
        soap_message.cc
        soap_response.cc
        soap_request.cc
        soap_request_handler.cc
        soap_xml.cc
        )

    set(HEADERS ${HEADERS} ${SOAP_HEADERS})
    set(SOURCES ${SOURCES} ${SOAP_SOURCES})
endif()

set(TARGET webcc)

add_library(${TARGET} STATIC ${HEADERS} ${SOURCES})

if(WEBCC_ENABLE_INSTALL)
    # Install lib and header files.
    # On Linux, if CMAKE_INSTALL_PREFIX is ~, the lib (libwebcc.a) will be installed
    # to ~/lib and header files will be installed to ~/include.
    install(TARGETS ${TARGET} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webcc)
endif()
