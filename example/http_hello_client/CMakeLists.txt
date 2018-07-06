add_executable(http_hello_client main.cc)

target_link_libraries(http_hello_client webcc ${Boost_LIBRARIES})
target_link_libraries(http_hello_client "${CMAKE_THREAD_LIBS_INIT}")
