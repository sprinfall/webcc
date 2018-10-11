add_executable(rest_github_client main.cc)

set(SSL_LIBS ${OPENSSL_LIBRARIES})
if(WIN32)
	set(SSL_LIBS ${SSL_LIBS} crypt32)
endif()

target_link_libraries(rest_github_client webcc jsoncpp ${Boost_LIBRARIES})
target_link_libraries(rest_github_client "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries(rest_github_client ${SSL_LIBS})
