set(LIBS webcc jsoncpp ${Boost_LIBRARIES} "${CMAKE_THREAD_LIBS_INIT}")

set(LIBS ${LIBS} ${OPENSSL_LIBRARIES})
if(WIN32)
	set(LIBS ${LIBS} crypt32)
endif()

if(UNIX)
	# Add `-ldl` for Linux to avoid "undefined reference to `dlopen'".
	set(LIBS ${LIBS} ${CMAKE_DL_LIBS})
endif()

add_executable(github_rest_client main.cc)

target_link_libraries(github_rest_client ${LIBS})
