# Don't use any deprecated definitions (e.g., io_service).
add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

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

if(WEBCC_ENABLE_SOAP)
	# SOAP specific sources.
	file(GLOB SOAP_SRCS soap_*.cc soap_*.h)
	set(SRCS ${SRCS} ${SOAP_SRCS})
endif()

add_library(webcc ${SRCS})
