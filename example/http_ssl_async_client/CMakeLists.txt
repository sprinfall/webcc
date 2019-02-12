add_executable(http_ssl_async_client main.cc)

# TODO
set(SSL_LIBS ${OPENSSL_LIBRARIES})
if(WIN32)
	set(SSL_LIBS ${SSL_LIBS} crypt32)
endif()

target_link_libraries(http_ssl_async_client webcc ${Boost_LIBRARIES})
target_link_libraries(http_ssl_async_client "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries(http_ssl_async_client ${SSL_LIBS})
