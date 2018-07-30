# Don't use any deprecated definitions (e.g., io_service).
add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(SRCS
	globals.cc
	globals.h
	http_async_client.cc
	http_async_client.h
	http_client.cc
	http_client.h
	http_connection.cc
	http_connection.h
	http_message.cc
	http_message.h
	http_parser.cc
	http_parser.h
	http_request.cc
	http_request.h
	http_request_handler.cc
	http_request_handler.h
	http_request_parser.cc
	http_request_parser.h
	http_response.cc
	http_response.h
	http_response_parser.cc
	http_response_parser.h
	http_server.cc
	http_server.h
	logger.cc
	logger.h
	queue.h
	rest_async_client.cc
	rest_async_client.h
	rest_client.cc
	rest_client.h
	rest_request_handler.cc
	rest_request_handler.h
	rest_service_manager.cc
	rest_service_manager.h
	rest_server.h
	rest_service.cc
	rest_service.h
	url.cc
	url.h
	utility.cc
	utility.h
	)

if(WEBCC_ENABLE_SSL)
	set(SRCS ${SRCS}
		http_ssl_client.cc
		http_ssl_client.h
	 	)
endif()

if(WEBCC_ENABLE_SOAP)
	# SOAP specific sources.
	set(SOAP_SRCS
		soap_client.cc
		soap_client.h
		soap_message.cc
		soap_message.h
		soap_parameter.h
		soap_response.cc
		soap_response.h
		soap_request.cc
		soap_request.h
		soap_request_handler.cc
		soap_request_handler.h
		soap_server.h
		soap_service.h
		soap_xml.cc
		soap_xml.h
		)

	set(SRCS ${SRCS} ${SOAP_SRCS})
endif()

add_library(webcc ${SRCS})
