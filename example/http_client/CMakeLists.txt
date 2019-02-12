add_executable(http_client main.cc)

target_link_libraries(http_client webcc ${Boost_LIBRARIES})
target_link_libraries(http_client "${CMAKE_THREAD_LIBS_INIT}")
