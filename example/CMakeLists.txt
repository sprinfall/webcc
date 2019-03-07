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

add_subdirectory(http_client)

if(WEBCC_ENABLE_REST)
	add_subdirectory(rest_book_server)
	# add_subdirectory(rest_book_client)
	add_subdirectory(github_client)
endif()

if(WEBCC_ENABLE_SOAP)
	add_subdirectory(soap_calc_server)
	add_subdirectory(soap_book_server)
	add_subdirectory(soap_book_client)
endif()

add_executable(soap_calc_client soap_calc_client.cc)
add_executable(soap_calc_client_parasoft soap_calc_client_parasoft.cc)

target_link_libraries(soap_calc_client ${EXAMPLE_COMMON_LIBS} pugixml)
target_link_libraries(soap_calc_client_parasoft ${EXAMPLE_COMMON_LIBS} pugixml)
