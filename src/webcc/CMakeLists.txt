option(WEBCC_DEBUG_OUTPUT "Enable debug output?" OFF)

if(WEBCC_DEBUG_OUTPUT)
    add_definitions(-DWEBCC_DEBUG_OUTPUT)
endif()

# Don't use any deprecated definitions (e.g., io_service).
add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

set(SRCS
	common.cc
	common.h
	http_client.cc
	http_client.h
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
	http_session.cc
	http_session.h
	queue.h
	rest_server.cc
	rest_server.h
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
