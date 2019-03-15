# Examples

# Common libraries to link for examples.
set(EXAMPLE_COMMON_LIBS webcc ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES}
	"${CMAKE_THREAD_LIBS_INIT}")
if(WIN32)
	set(EXAMPLE_COMMON_LIBS ${EXAMPLE_COMMON_LIBS} crypt32)
endif()

if(UNIX)
	# Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
	set(EXAMPLE_COMMON_LIBS ${EXAMPLE_COMMON_LIBS} ${CMAKE_DL_LIBS})
endif()

set(REST_BOOK_SRCS
	common/book.cc
	common/book.h
	common/book_json.cc
	common/book_json.h
	)

add_executable(http_client http_client.cc)
add_executable(github_client github_client.cc)

target_link_libraries(http_client ${EXAMPLE_COMMON_LIBS})
target_link_libraries(github_client ${EXAMPLE_COMMON_LIBS} jsoncpp)

add_executable(rest_book_server rest_book_server.cc ${REST_BOOK_SRCS})
add_executable(rest_book_client rest_book_client.cc ${REST_BOOK_SRCS})

target_link_libraries(rest_book_server ${EXAMPLE_COMMON_LIBS} jsoncpp)
target_link_libraries(rest_book_client ${EXAMPLE_COMMON_LIBS} jsoncpp)

if(WEBCC_ENABLE_SOAP)
	add_executable(soap_calc_server soap_calc_server.cc)
	add_executable(soap_calc_client soap_calc_client.cc)
	add_executable(soap_calc_client_parasoft soap_calc_client_parasoft.cc)

	target_link_libraries(soap_calc_server ${EXAMPLE_COMMON_LIBS} pugixml)
	target_link_libraries(soap_calc_client ${EXAMPLE_COMMON_LIBS} pugixml)
	target_link_libraries(soap_calc_client_parasoft ${EXAMPLE_COMMON_LIBS} pugixml)

	set(SOAP_BOOK_SRCS
		common/book.cc
		common/book.h
		common/book_xml.cc
		common/book_xml.h
		)

	add_executable(soap_book_server soap_book_server.cc ${SOAP_BOOK_SRCS})
	add_executable(soap_book_client soap_book_client.cc ${SOAP_BOOK_SRCS})

	target_link_libraries(soap_book_server ${EXAMPLE_COMMON_LIBS} pugixml)
	target_link_libraries(soap_book_client ${EXAMPLE_COMMON_LIBS} pugixml)
endif()
